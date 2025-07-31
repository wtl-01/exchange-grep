from pyexpat import model
import numpy as np
import pandas as pd
import tensorflow as tf
import os
import sys
import re
from tensorflow.keras.models import Model
from tensorflow.keras.layers import Input, Conv1D, LSTM, Dense, Concatenate, LeakyReLU, Dropout, MaxPooling1D
from tensorflow.keras.callbacks import EarlyStopping
from tensorflow.keras.optimizers import Adam

from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, matthews_corrcoef
import warnings
from sklearn.datasets import make_classification
import math

# Suppress warnings for cleaner output
warnings.filterwarnings('ignore')


# --- 1. Data Loading and Preprocessing ---

def load_and_preprocess_data(filepath, tick_size=0.05, levels=10):
    """
    Loads and preprocesses the Level 2 data from a Parquet file.
    
    Args:
        filepath (str): The path to the Parquet data file.
        tick_size (float): The minimum price movement for the instrument.
        levels (int): The number of order book levels to use.

    Returns:
        tuple: A tuple containing the raw feature array, the mid-price series, and the timestamps.
    """
    print(f"Loading data from {filepath}...")
    df = pd.read_parquet(filepath)
    
    if 'srcTime' in df.columns and not isinstance(df.index, pd.DatetimeIndex):
        df.set_index('srcTime', inplace=True)
    df.sort_index(inplace=True)

    print("Data loaded. Preprocessing...")
    df['mid_price'] = (df['bestBidPx'] + df['bestAskPx']) / 2.0

    feature_cols = []
    for level in range(levels):
        # Synthetically create price levels
        df[f'ask_price_{level}'] = df['bestAskPx'] + (level * tick_size)
        df[f'bid_price_{level}'] = df['bestBidPx'] - (level * tick_size)
        
        # Define the column order for features
        feature_cols.extend([f'ask_price_{level}', f'ask_qty_{level}', f'bid_price_{level}', f'bid_qty_{level}'])

    for col in feature_cols:
        if col not in df.columns:
            raise ValueError(f"Column '{col}' not found in DataFrame. Please check your data schema and file.")

    # Use float32 to reduce memory usage by half
    features = df[feature_cols].values.astype(np.float32)
    mid_price = df['mid_price'].values.astype(np.float32)
    
    print("Preprocessing complete.")
    return features, mid_price, df.index

# --- 2. Label Creation ---

def create_labels(mid_price, k=50, alpha=0.002):
    """
    Creates classification labels based on future mid-price movements.
    """
    price_series = pd.Series(mid_price)
    future_avg = price_series.rolling(window=k, min_periods=1).mean().shift(-k)
    past_avg = price_series.rolling(window=k, min_periods=1).mean()
    
    pct_change = (future_avg - past_avg) / past_avg
    
    labels = np.full(len(mid_price), 1, dtype=np.int8) # Use memory-efficient integer type
    labels[pct_change < -alpha] = 0
    labels[pct_change > alpha] = 2
    
    # Handle NaNs that result from the look-ahead shift
    labels[pd.isna(pct_change)] = -1 # Use -1 as a sentinel for invalid labels
    
    return labels

# --- 3. Memory-Efficient Data Pipeline ---

def create_tf_dataset(features, labels, sequence_length=100, batch_size=256):
    """
    Creates a tf.data.Dataset pipeline for memory-efficient training.
    This replaces the memory-intensive create_sequences function.
    """
    # Remove samples with invalid labels
    valid_indices = np.where(labels != -1)[0]
    features = features[valid_indices]
    labels = labels[valid_indices]

    # Create a dataset from the raw features and labels
    dataset = tf.data.Dataset.from_tensor_slices((features, labels))

    # Use the window method to create overlapping sequences
    dataset = dataset.window(size=sequence_length, shift=1, drop_remainder=True)
    
    # Flatten the windows into tensors
    dataset = dataset.flat_map(lambda x, y: tf.data.Dataset.zip((
        x.batch(sequence_length),
        y.batch(sequence_length)
    )))

    # Map to the final (X, y) structure where y is the last label in the sequence
    dataset = dataset.map(lambda x, y: (x, y[-1]))
    
    # Batch, prefetch for performance
    dataset = dataset.batch(batch_size).prefetch(tf.data.AUTOTUNE)
    
    return dataset


def build_deeplob_model(input_shape=(100, 40), num_classes=3):
    input_tensor = Input(shape=input_shape, dtype=tf.float32)

    conv_block = Conv1D(filters=16, kernel_size=2, strides=2, padding='same')(input_tensor)
    conv_block = LeakyReLU(alpha=0.01)(conv_block)
    conv_block = Conv1D(filters=16, kernel_size=2, strides=2, padding='same')(conv_block)
    conv_block = LeakyReLU(alpha=0.01)(conv_block)
    conv_block = Conv1D(filters=16, kernel_size=10, padding='same')(conv_block)
    conv_block = LeakyReLU(alpha=0.01)(conv_block)

    tower_1 = Conv1D(filters=32, kernel_size=1, padding='same', activation=LeakyReLU(alpha=0.01))(conv_block)
    tower_1 = Conv1D(filters=32, kernel_size=3, padding='same', activation=LeakyReLU(alpha=0.01))(tower_1)

    tower_2 = Conv1D(filters=32, kernel_size=1, padding='same', activation=LeakyReLU(alpha=0.01))(conv_block)
    tower_2 = Conv1D(filters=32, kernel_size=5, padding='same', activation=LeakyReLU(alpha=0.01))(tower_2)
    
    tower_3 = MaxPooling1D(pool_size=3, strides=1, padding='same')(conv_block)
    tower_3 = Conv1D(filters=32, kernel_size=1, padding='same', activation=LeakyReLU(alpha=0.01))(tower_3)

    inception_output = Concatenate(axis=-1)([tower_1, tower_2, tower_3])

    lstm_output = LSTM(64)(inception_output)
    lstm_output = Dropout(0.25)(lstm_output)

    output = Dense(num_classes, activation='softmax')(lstm_output)

    model = Model(inputs=input_tensor, outputs=output)
    custom_optimizer = Adam(learning_rate=0.0001) 
    model.compile(optimizer=custom_optimizer, loss='sparse_categorical_crossentropy', metrics=['accuracy'])
    
    return model


if __name__ == '__main__':
    
    SEQUENCE_LENGTH = 100
    PREDICTION_HORIZON = 50
    LABEL_THRESHOLD = 0.001
    BATCH_SIZE = 256
    FPATH_RAW = 'processed_data/filtered_vxv4_trading.parquet'
    CWD_DIR = os.getcwd()
    DATA_FILEPATH = os.path.join(CWD_DIR, FPATH_RAW)
    
    try:
        features, mid_price, timestamps = load_and_preprocess_data(DATA_FILEPATH)
    except FileNotFoundError:
        print(f"Error: Data file not found at '{DATA_FILEPATH}'.")
        print("Generating synthetic data for demonstration purposes...")
        features = np.random.rand(50000, 40).astype(np.float32)
        mid_price = (100 + np.random.randn(50000).cumsum() * 0.1).astype(np.float32)
        timestamps = pd.to_datetime(pd.date_range(start='2024-01-01', periods=50000, freq='s'))
        
    print(f"\nTotal LOB snapshots loaded: {len(features)}")
    
    print("Creating labels for the entire dataset...")
    labels = create_labels(mid_price, k=PREDICTION_HORIZON, alpha=LABEL_THRESHOLD)

    data_df = pd.DataFrame(features, index=timestamps)
    data_df['label'] = labels
    data_df['day'] = data_df.index.date

    unique_days = sorted(data_df['day'].unique())
    if len(unique_days) < 2:
        raise ValueError("At least 2 days of data are required for walk-forward validation.")

    all_y_true = []
    all_y_pred = []

    print("\nStarting Walk-Forward Validation...")
    for i in range(1, len(unique_days)):
        train_days = unique_days[:i]
        test_day = unique_days[i]
        
        print(f"\n--- Fold {i}/{len(unique_days)-1}: Training on days {train_days}, Testing on day {test_day} ---")
        
        train_df = data_df[data_df['day'].isin(train_days)]
        test_df = data_df[data_df['day'] == test_day]

        train_features = train_df.drop(columns=['label', 'day']).values
        train_labels = train_df['label'].values
        test_features = test_df.drop(columns=['label', 'day']).values
        test_labels = test_df['label'].values
        
        scaler = StandardScaler()
        scaler.fit(train_features)
        train_features_scaled = scaler.transform(train_features)
        test_features_scaled = scaler.transform(test_features)
        
        val_split_index = int(len(train_features_scaled) * 0.9)
        
        fold_X_train, fold_y_train = train_features_scaled[:val_split_index], train_labels[:val_split_index]
        fold_X_val, fold_y_val = train_features_scaled[val_split_index:], train_labels[val_split_index:]

        train_dataset = create_tf_dataset(fold_X_train, fold_y_train, SEQUENCE_LENGTH, BATCH_SIZE)
        val_dataset = create_tf_dataset(fold_X_val, fold_y_val, SEQUENCE_LENGTH, BATCH_SIZE)
        test_dataset = create_tf_dataset(test_features_scaled, test_labels, SEQUENCE_LENGTH, BATCH_SIZE)
        

        if tf.data.experimental.cardinality(train_dataset).numpy() == 0 or tf.data.experimental.cardinality(test_dataset).numpy() == 0:
            print("Skipping fold due to insufficient data to create sequences.")
            continue
            
        model = build_deeplob_model(input_shape=(SEQUENCE_LENGTH, features.shape[1]))
        if i == 1:
            model.summary()
        
        # Calculate class weights from the training portion only
        unique, counts = np.unique(fold_y_train[fold_y_train != -1], return_counts=True)
        class_weights = {
            cls: len(fold_y_train) / (len(unique) * count) for cls, count in zip(unique, counts)
        }
        print(f"Class weights for training: {class_weights}")

        early_stopping = EarlyStopping(monitor='val_loss', patience=15, restore_best_weights=True)

        model.fit(train_dataset, 
                  epochs=60, 
                  validation_data=val_dataset,
                  class_weight=class_weights,
                  callbacks=[early_stopping],
                  verbose=2)

        print("Predicting on test set...")
        y_pred_probs = model.predict(test_dataset)
        y_pred = np.argmax(y_pred_probs, axis=1)
        
        # Extract true labels from the test dataset
        y_true = np.concatenate([y for x, y in test_dataset], axis=0)

        all_y_true.extend(y_true)
        all_y_pred.extend(y_pred)
        
        print(f"\nFold {i} Performance Report:")
        print(classification_report(y_true, y_pred, target_names=['Down', 'Stationary', 'Up'], zero_division=0))
        print(f"Fold {i} Matthews Correlation Coefficient: {matthews_corrcoef(y_true, y_pred):.4f}")

    if all_y_true:
        print("\n--- Final Aggregated Performance Across All Folds ---")
        print(classification_report(all_y_true, all_y_pred, target_names=['Down', 'Stationary', 'Up'], zero_division=0))
        mcc = matthews_corrcoef(all_y_true, all_y_pred)
        print(f"\nOverall Matthews Correlation Coefficient: {mcc:.4f}")

        # output model and save it
        #model_save_path = 'deeplob_model_vxv4.h5'
        #model.save(model_save_path)

        MFPATH = 'deeplob_model_vxv4.keras'
        MODEL_FILEPATH = os.path.join(CWD_DIR, MFPATH)
        model.save(MODEL_FILEPATH)
        print(f"\nModel saved to {MODEL_FILEPATH}")
    else:
        print("\nNo predictions were made. Please check configuration.")

