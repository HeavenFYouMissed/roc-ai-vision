[![Hugging Face's logo](https://huggingface.co/front/assets/huggingface_logo-noborder.svg)Hugging Face](https://huggingface.co/)

- [Models](https://huggingface.co/models)
- [Datasets](https://huggingface.co/datasets)
- [Spaces](https://huggingface.co/spaces)
- [Buckets new](https://huggingface.co/storage)
- [Docs](https://huggingface.co/docs)
- [Enterprise](https://huggingface.co/enterprise)
- [Pricing](https://huggingface.co/pricing)
- - Website

    - [Tasks](https://huggingface.co/tasks)
    - [HuggingChat](https://huggingface.co/chat)
    - [Collections](https://huggingface.co/collections)
    - [Languages](https://huggingface.co/languages)
    - [Organizations](https://huggingface.co/organizations)
  - Community

    - [Blog](https://huggingface.co/blog)
    - [Posts](https://huggingface.co/posts)
    - [Daily Papers](https://huggingface.co/papers)
    - [Learn](https://huggingface.co/learn)
    - [Discord](https://huggingface.co/join/discord)
    - [Forum](https://discuss.huggingface.co/)
    - [GitHub](https://github.com/huggingface)
  - Solutions

    - [Team & Enterprise](https://huggingface.co/enterprise)
    - [Hugging Face PRO](https://huggingface.co/pro)
    - [Enterprise Support](https://huggingface.co/support)
    - [Inference Providers](https://huggingface.co/inference/models)
    - [Inference Endpoints](https://huggingface.co/inference-endpoints)
    - [Storage Buckets](https://huggingface.co/storage)

- * * *

- [Log In](https://huggingface.co/login)
- [Sign Up](https://huggingface.co/join)

# [Datasets:](https://huggingface.co/datasets)    [![](https://www.gravatar.com/avatar/73e994be26ad0909c88d0fca3185388b?d=retro&size=100)](https://huggingface.co/beacon-gui)  [beacon-gui](https://huggingface.co/beacon-gui)  /      [BEACON-Dataset](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset)    like3           Follow ![](https://www.gravatar.com/avatar/73e994be26ad0909c88d0fca3185388b?d=retro&size=100)The BEACON Project4

Modalities: [Video](https://huggingface.co/datasets?modality=modality%3Avideo)

ArXiv:

arxiv:2605.10867

DOI:

doi:10.57967/hf/8674

[Dataset card](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset) [Data Studio](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset/viewer/) [FilesFiles and versions\\
xet](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset/tree/main) [Community](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset/discussions)

## You need to agree to share your contact information to access this dataset

This repository is publicly accessible, but you have to accept the conditions to access its files and content.

[Log in](https://huggingface.co/login?next=%2Fdatasets%2Fbeacon-gui%2FBEACON-Dataset) or [Sign Up](https://huggingface.co/join?next=%2Fdatasets%2Fbeacon-gui%2FBEACON-Dataset) to review the conditions and access this dataset content.

**YAML Metadata**
**Warning:** empty or missing yaml metadata in repo card

Check out the [documentation](https://huggingface.co/docs/hub/datasets-cards) for more information.

- [Modalities](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#modalities "Modalities")
- [File Layout](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#file-layout "File Layout")
- [Metadata](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#metadata "Metadata")
- [Anonymization](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#anonymization "Anonymization")
- [Citation](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#citation "Citation")
- [License](https://huggingface.co/datasets/beacon-gui/BEACON-Dataset#license "License")

# BEACON: Behavioral Engine for Authentication and Continuous Monitoring

An anonymized multi-modal behavioral biometric dataset collected during VALORANT gameplay,
designed for continuous user authentication research.

**28 participants · 79 sessions · 445 GB**

Participants are represented by opaque IDs (`P001`–`P028`) and sessions by sequential
IDs (`S001`, `S002`, ...). Raw participant names, consent files, hostnames, MAC
addresses, and direct device IDs are not included in this release.

## Modalities

| Modality | Files | Description |
| --- | --- | --- |
| `mouse_*.csv` | 80 | Mouse telemetry — position, speed, clicks, scroll; Unix epoch timestamps |
| `keyboard_*.csv` | 80 | Keyboard HID dynamics — key identity, press/release times, duration; Unix epoch timestamps |
| `captured_packets_*.pcap` | 80 | Network traffic — IP/MAC addresses anonymized, local-network discovery traffic removed, encrypted game payload preserved |
| `screen_record_*.mp4` | 74 | Screen recordings of gameplay sessions |
| `hardware_info_*.json` | 79 | Scrubbed hardware metadata (hostname, MAC, IP, serial removed) |

Sessions with multiple recording parts are named `screen_record_P###_S###_part02.mp4` etc.

## File Layout

```
participant_P001/
  session_S001/
    mouse_P001_S001.csv
    keyboard_P001_S001.csv
    captured_packets_P001_S001.pcap
    screen_record_P001_S001.mp4
    hardware_info_P001_S001.json
```

## Metadata

| File | Description |
| --- | --- |
| `metadata/public_participant_session_manifest.csv` | Participant and session ID index |
| `metadata/public_release_manifest.csv` | Per-file modality, status, and size |
| `metadata/sha256_checksums.csv` | SHA-256 checksums for all included files |
| `metadata/beacon_croissant.jsonld` | Croissant ML metadata record |

## Anonymization

- Participant and session folders use opaque sequential IDs
- CSV timestamps are raw Unix epoch values (not session-relative)
- PCAP: Ethernet/IP src and dst addresses replaced with deterministic pseudonyms
in `10.0.0.0/8`; local-network broadcast/discovery UDP (mDNS, LLMNR, NBNS,
SSDP, DHCP, WSD) dropped to prevent hostname leakage; encrypted game payload
and regular DNS preserved intact
- Hardware JSON: hostname, username, MAC, IP, HID, serial, UUID fields removed
- Raw consent files retained privately; see `consent_policy.md`

## Citation

If you use this dataset, please cite it:

Singh, I., Kaur, G., Atwal, U. P. S., Singh, G., Singh, G., & Singh, M. (2026).
BEACON: A Multimodal Dataset for Learning Behavioral Fingerprints from Gameplay Data
\[Data set\]. Zenodo. [https://doi.org/10.5281/zenodo.20034625](https://doi.org/10.5281/zenodo.20034625) [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.20034625.svg)](https://doi.org/10.5281/zenodo.20034625)

Singh, I., Singh, G., Kaur, G., Atwal, U. P. S., Singh, G., & Singh, M. (2026).
BEACON-Logger: A Behavioral Authentication and Network Traffic Logger for Game Environments.
Zenodo. [https://doi.org/10.5281/zenodo.20062628](https://doi.org/10.5281/zenodo.20062628) [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.20144661.svg)](https://doi.org/10.5281/zenodo.20144661)

Please cite the following paper also:

```bibtex
@misc{singh2026beaconmultimodaldatasetlearning,
      title={BEACON: A Multimodal Dataset for Learning Behavioral Fingerprints from Gameplay Data},
      author={Ishpuneet Singh and Gursmeep Kaur and Uday Pratap Singh Atwal and Guramrit Singh and Gurjot Singh and Maninder Singh},
      year={2026},
      eprint={2605.10867},
      archivePrefix={arXiv},
      primaryClass={cs.CR},
      url={https://arxiv.org/abs/2605.10867},
}
```

## License

Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)

Downloads last month

73

Total file size:

905 GB

## Paper for beacon-gui/BEACON-Dataset

[**BEACON: A Multimodal Dataset for Learning Behavioral Fingerprints from Gameplay Data**\\
Paper • 2605.10867 •Published 6 days ago• 5](https://huggingface.co/papers/2605.10867)

System theme

Company

[TOS](https://huggingface.co/terms-of-service) [Privacy](https://huggingface.co/privacy) [About](https://huggingface.co/huggingface) [Careers](https://apply.workable.com/huggingface/)  [Hugging Face](https://huggingface.co/)

Website

[Models](https://huggingface.co/models) [Datasets](https://huggingface.co/datasets) [Spaces](https://huggingface.co/spaces) [Pricing](https://huggingface.co/pricing) [Docs](https://huggingface.co/docs)