[About](https://www.easy.ac/) [About](https://www.easy.ac/) [Support](https://www.easy.ac/support) [Licensing](https://www.easy.ac/licensing)

[About](https://www.easy.ac/) [Support](https://www.easy.ac/support) [Licensing](https://www.easy.ac/licensing)

- [Account Bans](https://www.easy.ac/support/articles/account-bans)
- Additional Security Requirements
- [Collecting system logs](https://www.easy.ac/support/articles/collecting-system-logs)
- [EAC Windows Service](https://www.easy.ac/support/articles/eac-windows-service)
- [Error: Easy Anti-Cheat is not installed](https://www.easy.ac/support/articles/error-eac-is-not-installed)
- [Error: Maximum simultaneous game instances reached!](https://www.easy.ac/support/articles/error-maximum-simultaneous-instances)
- [Error: Unknown file version: <>](https://www.easy.ac/support/articles/error-unknown-file-version)
- [Error 20010: Easy Anti-Cheat installation folder has been encrypted](https://www.easy.ac/support/articles/error-20010)
- [Error 30005](https://www.easy.ac/support/articles/error-30005)

# Additional Security Requirements

Games using EAC may impose additional security requirements on players which are controlled by the developer. These requirements might apply to specific game types only (like competitive), specific accounts only (like those considered high risk), or might apply to all players. Check the information provided by the specific game you are playing for details. This page provides general information about the available requirements and troubleshooting steps if you have problems meeting them.

## IMPORTANT

The troubleshooting steps below involve editing your BIOS settings and doing so incorrectly might lead to issues such as preventing your computer from correctly starting. If you're not comfortable doing this yourself please reach out to an expert.

Also note that BIOS varies a lot depending on the manufacturer and version. The suggestions below are high-level guidelines and we recommend that you refer to your motherboard manufacturers support resources for the specifics.

## Secure Boot

This is a security feature that helps prevent malware from being inserted into the PC’s boot process. Some cheats attempt to insert themselves into the boot process in the same way and are blocked by Secure Boot.

For suggestions on how to determine whether your PC is Secure Boot capable and if so to enable it, see this page:

[Windows 11 and Secure Boot - Microsoft Support](https://support.microsoft.com/windows/windows-11-and-secure-boot-a8ff1202-c0d9-42f5-940f-843abef64fad)

## TPM / Cryptographic Processor

This is a special hardware component that can provide support for cryptographic operations like encryption and decryption. It can also be used to verify that certain other security settings like Secure Boot are enabled in a way that is very difficult for malware or cheats to tamper with. In older machines this was sometimes a separate chip on the motherboard, but for newer ones it is usually a part of the CPU itself.

Most PCs produced since 2016 include a TPM and having one is part of the minimum requirements for Windows 11, but the TPM may need to be enabled by changing system settings. For suggestions on how to determine whether your PC has a TPM and if so to enable it, see this page:

[Enable TPM 2.0 on your PC - Microsoft Support](https://support.microsoft.com/windows/enable-tpm-2-0-on-your-pc-1fd5a332-360d-4f46-a1e7-ae6b0c90645c)

If you're having issues please open PowerShell with administrative privileges and:

- Run the command `tpmtool getdeviceinformation` and make sure:
  - `TPM Present` is true.
  - `TPM Version` is 2.0.
  - `Is Initialized` is true.
  - `Ready For Attestation`is true.
    - If this is false, `Information Flags Description` usually has some hints on what's missing.
- Run the command `get-tpm` and make sure:
  - `TpmPresent` is true.
  - `TpmReady` is true.
  - `TpmEnabled` is true.
  - `TpmOwned` is true.
  - `AutoProvisioning`is true.
    - If this is false, you can enable it by running "Enable-TpmAutoProvisioning" in the elevated PowerShell window and restarting the machine.

If any of the above fields are not set properly please make sure:

- That you're on a supported version of Windows and have the most recent updates installed.
- That you're on a recent BIOS version. Some CPU and BIOS combinations have known issues that cause the TPM to not work correctly and these are sometimes fixed by newer BIOS versions.

If neither of the above steps helped please reach out to your TPM manufacturer for further assistance.

## IOMMU

This is a hardware feature that gives the operating system more control over how peripheral devices like PCI cards can access system memory. Some cheats are implemented with hardware cards that use Direct Memory Access (DMA) to make game memory available to a second PC which runs cheat software. IOMMU support can help prevent these cheat devices from working.

Most PCs produced since 2010 include IOMMU support, but it may need to be enabled by changing system settings. In your system settings, IOMMU may also be called Intel VT-d or AMD-Vi. Check the manual for your PC or motherboard to help find the correct setting and how to enable it.

## Kicks Troubleshooting

### **Boot validation failed: Measured bootlog not found.**

Windows has not generated (or it has been removed post-boot) the measured bootlog that we require to validate the state of your machine. Please make sure both TPM & Secure Boot is properly enabled on your system.

### **Boot validation failed: PCR mismatch.**

This means that the events in your bootlog are not the same as was measured and extended into the TPM during boot. Something likely removed, modified or deleted events in the bootlog OR extended the tpm PCR without adding the event to the bootlog.

### **Unable to verify Secure boot. Event missing from measured bootlog.**

This means that there's no event in the measured bootlog confirming that secure boot is enabled. The event should be added by your firmware at boot, please make sure you're running the latest BIOS version for your system.

[Licensing](https://www.easy.ac/licensing) [Support](https://www.easy.ac/support)

[Epic Games Store](https://www.epicgames.com/store) [Services](https://dev.epicgames.com/services) [Unreal Engine](https://www.unrealengine.com/)

Back to top

© 2004-2024, Epic Games, Inc. All rights reserved. Easy Anti-Cheat and its logo are Epic's trademarks or registered trademarks in the US and elsewhere.

We expect everyone to follow our rules and guidelines to keep the Epic Games ecosystem safe and fun for everyone. We encourage everyone to report potentially inappropriate content and conduct. If you would like to submit a report, you can do so [here](https://safety.epicgames.com/en-US/policies/reporting-misconduct/submit-report?product_id=eac_content_report).

[Terms of Service](https://www.epicgames.com/site/tos) [Privacy Policy](https://www.epicgames.com/site/privacypolicy)

![eac logo](https://www.easy.ac/_next/static/media/logo-easy-typography-white.d0a57c33.svg)