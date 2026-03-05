#!/usr/bin/env python3
"""
Hardware Wallet ESP32 - Desktop Companion
CLI para crear PSBT y comunicarse con la wallet
"""

import argparse
import json
import os
import sys

def create_psbt():
    """Create unsigned PSBT transaction"""
    print("Creating PSBT transaction...")
    recipient = input("Recipient address: ")
    amount = input("Amount (sats): ")
    
    psbt = {
        "version": 0,
        "recipient": recipient,
        "amount": int(amount),
        "unsigned": True
    }
    
    filename = f"tx_{int(amount)}_unsigned.json"
    with open(filename, 'w') as f:
        json.dump(psbt, f, indent=2)
    
    print(f"✓ PSBT saved: {filename}")
    print(f"  Copy to SD card and insert in wallet")

def load_signed():
    """Load signed PSBT"""
    print("Loading signed PSBT...")
    # Implementation reads from SD card
    print("Looking for signed transactions in /media/sd...")

def show_address():
    """Display wallet address"""
    print("Wallet address: bc1q...")
    print("QR Code:")

def main():
    parser = argparse.ArgumentParser(description='Hardware Wallet ESP32 Desktop Companion')
    parser.add_argument('command', choices=['create', 'sign', 'address'])
    
    args = parser.parse_args()
    
    if args.command == 'create':
        create_psbt()
    elif args.command == 'sign':
        load_signed()
    elif args.command == 'address':
        show_address()

if __name__ == '__main__':
    main()
