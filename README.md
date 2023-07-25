# ezUnpacker
This project aims to decrypt the executable statically (without dumping) and fix 
various things to ensure correct analysis in RE softwares.

# Instructions
After compiling the solution you'll need 2 things:
- Put the encrypted executable into the directory in which ezUnpacker.exe is
- Provide a good 'data.json' file, which is crucial for the whole decrpyption process.

# data.json
This file contains RVAs and INTs that will be used to decrypt the content. Here's an example of how it looks like:

```json
{
  "tokenBlock": "0x3958E7F",
  "encryptedDataBlock": "0x3958E7F",
  "decryptionKeyRot": "0xCC5C4F",
  "decryptionKeyRot2": "0x61C88647",
  "decryptionKeyRot3": "0x0F703ED3B",
  "decryptionKey": "0x3322308",
  "tokenCheck": "0x0CEB4E63",
  "keyCheck": "0xFE688B47"
}
```

This file shall be changed after every update with the corresponding values. There's no practical
way of retrieving them statically too, unless you make a complex heuristic checker, because the 
heavy obfuscation within all decryptor itself.

# Remarks
This software will only work with *GTA5*, other games using the same protector don't have the same
scheme as it, so it'll surely fail when creating the new OEP.