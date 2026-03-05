#!/usr/bin/env python3
"""
PSBT Builder for Hardware Wallet ESP32
"""

class PSBTBuilder:
    """Builds PSBT transactions for signing"""
    
    def __init__(self):
        self.version = 0
        self.inputs = []
        self.outputs = []
    
    def add_input(self, txid: str, vout: int, amount: int):
        """Add an input UTXO"""
        self.inputs.append({
            'txid': txid,
            'vout': vout,
            'amount': amount
        })
    
    def add_output(self, address: str, amount: int):
        """Add an output"""
        self.outputs.append({
            'address': address,
            'amount': amount
        })
    
    def build(self) -> dict:
        """Build the unsigned PSBT"""
        return {
            'version': self.version,
            'inputs': self.inputs,
            'outputs': self.outputs,
            'unsigned': True
        }

# Example usage
if __name__ == '__main__':
    builder = PSBTBuilder()
    builder.add_input('abc123...', 0, 100000)
    builder.add_output('bc1q...', 50000)
    print(builder.build())
