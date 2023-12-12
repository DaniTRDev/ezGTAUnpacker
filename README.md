# OUTDATED
I'll post the new 'data.json' as soon as I can.

# ezUnpacker
This project aims to decrypt the executable statically (without dumping) and fix 
various things to ensure correct analysis in RE softwares.

# Instructions
After compiling the solution you'll need to do 2 things:
- Rename the executable to 'encrypted.exe'.
- Put the encrypted executable into the directory in which ezUnpacker.exe is.
- Provide a good 'data.json' file, which is crucial for the whole decryption process.

# data.json
This file contains RVAs and INTs that will be used to decrypt the content. Here's an example of how it looks like (1.67, 3028):

```json
{
  "tokenBlock": "0x3AE76E3",
  "encryptedDataBlock": "0x3AE76E3",
  "decryptionKeyRot": "0x39EFF7F",
  "decryptionKeyRot2": "0x61C88647",
  "decryptionKey": "0x34FC8C5",
  "tokenCheck": "0x0D4B6CD6E"
}
```

This file shall be changed after every update with the corresponding values because there's no practical
way of retrieving them statically, unless you make a complex heuristic checker that can skip over the 
heavy obfuscation within all the decryptor itself.

# Remarks
This software will only work with *GTA5*, other games using the same protector don't have the same scheme as it, so it'll surely fail when creating the new OEP.
This project is not intended to be used as a first step to crack the game or any other thing involved in infrighting their copyright. Every other protection given (at the time of publishing this) by Arxan will still remain in the decrypted file and the generated file *WON'T* run as if it was a normal .exe. To ensure softwares like IDA or GHIDRA scan this file better, new pages permissions are set in the PE header, making the program fail in some checks and eventually crashing. 

# Disclaimer
Please read the following disclaimer carefully before using ezUnpacker for Grand Theft Auto V (GTA5):

ezUnpacker is a software tool created by DaniTRDev with the primary purpose of facilitating the unpacking and extraction of certain files from the game Grand Theft Auto V. This tool is intended to be used by individuals who have legal access to the game and its files, either through a legitimate purchase or other authorized means.

Limited Purpose: ezUnpacker is specifically designed for use with Grand Theft Auto V and is solely intended for the purpose of extracting and unpacking files associated with this game.

Non-Commercial Use: The Author provides ezUnpacker free of charge and does not intend to profit from its distribution or usage. The tool is shared with the community as a convenience, and no financial transactions should occur for obtaining or using this software.

Ownership: Grand Theft Auto V is the intellectual property of its respective developers and publishers. The Author acknowledges that all rights, titles, and interests in and to Grand Theft Auto V, including any associated trademarks and copyrights, are the property of their respective owners.

Legal Compliance: Users of ezUnpacker are responsible for complying with all applicable copyright laws and regulations. The tool should only be used for legal purposes, such as modding or personal use, where permitted by the law.

No Warranty: ezUnpacker is provided "as is," without any warranty or guarantee of any kind, expressed or implied. The Author will not be liable for any damage or loss caused by the use of this tool.

No Endorsement: The Author's distribution of ezUnpacker does not imply any endorsement, sponsorship, or approval from the developers or publishers of Grand Theft Auto V.

Usage Risks: Users should be aware that using third-party tools like ezUnpacker could potentially lead to unintended consequences, including but not limited to data corruption, game instability, or even penalties from the game's developers for violating the terms of service.

By using ezUnpacker, you agree to the terms and conditions outlined in this disclaimer. If you do not agree with any part of this disclaimer, you must not use ezUnpacker.

Please use ezUnpacker responsibly and respect the intellectual property rights of others. The Author shall not be held responsible for any misuse or unauthorized use of ezUnpacker.
