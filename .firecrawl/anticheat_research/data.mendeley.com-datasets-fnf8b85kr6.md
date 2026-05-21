[Skip to main content](https://data.mendeley.com/datasets/fnf8b85kr6/1#main)

[Mendeley Data homepage](https://data.mendeley.com/)

- [FAQ](https://data.mendeley.com/faq "FAQ")

Sign In / Register

Sign In / Register

- [FAQ](https://data.mendeley.com/faq "FAQ")

# Behaviour Biometrics Dataset

Published: 20 June 2022\| Version 1 \| DOI: 10.17632/fnf8b85kr6.1

Contributors:

Nonso Nnamoko,

Joe Barrowclough

,

Mark Liptrott

,

Ioannis Korkontzelos

## Description

The dataset provides a collection of behaviour biometrics data (commonly known as Keyboard, Mouse and Touchscreen (KMT) dynamics). The data was collected for use in a FinTech research project undertaken by academics and researchers at Computer Science Department, Edge Hill University, United Kingdom. The project called CyberSIgnature uses KMT dynamics data to distinguish between legitimate card owners and fraudsters. An application was developed that has a graphical user interface (GUI) similar to a standard online card payment form including fields for card type, name, card number, card verification code (cvc) and expiry date. Then, user KMT dynamics were captured while they entered fictitious card information on the GUI application.

The dataset consists of 1,760 KMT dynamic instances collected over 88 user sessions on the GUI application. Each user session involves 20 iterations of data entry in which the user is assigned a fictitious card information (drawn at random from a pool) to enter 10 times and subsequently presented with 10 additional card information, each to be entered once. The 10 additional card information is drawn from a pool that has been assigned or to be assigned to other users. A KMT data instance is collected during each data entry iteration. Thus, a total of 20 KMT data instances (i.e., 10 legitimate and 10 illegitimate) was collected during each user entry session on the GUI application.

The raw dataset is stored in .json format within 88 separate files. The root folder named \`behaviour\_biometrics\_dataset' consists of two sub-folders \`raw\_kmt\_dataset' and \`feature\_kmt\_dataset'; and a Jupyter notebook file (kmt\_feature\_classificatio.ipynb). Their folder and file content is described below:

\-\- \`raw\_kmt\_dataset': this folder contains 88 files, each named \`raw\_kmt\_user\_n.json', where n is a number from 0001 to 0088. Each file contains 20 instances of KMT dynamics data corresponding to a given fictitious card; and the data instances are equally split between legitimate (n = 10) and illegitimate (n = 10) classes. The legitimate class corresponds to KMT dynamics captured from the user that is assigned to the card detail; while the illegitimate class corresponds to KMT dynamics data collected from other users entering the same card detail.

\-\- \`feature\_kmt\_dataset': this folder contains two sub-folders, namely: \`feature\_kmt\_json' and \`feature\_kmt\_xlsx'. Each folder contains 88 files (of the relevant format: .json or .xlsx) , each named \`feature\_kmt\_user\_n', where n is a number from 0001 to 0088. Each file contains 20 instances of features extracted from the corresponding \`raw\_kmt\_user\_n' file including the class labels (legitimate = 1 or illegitimate = 0).

\-\- \`kmt\_feature\_classification.ipynb': this file contains python code necessary to generate features from the raw KMT files and apply simple machine learning classification task to generate results. The code is designed to run with minimal effort from the user.

[Download All 24.2 MB](https://data.mendeley.com/public-api/zip/fnf8b85kr6/download/1)

Behaviour Biometrics Dataset.zip

SHA-256 checksum:

Copy Checksum

## Files

behaviour\_biometrics\_dataset

## Steps to reproduce

An application was developed that has a graphical user interface (GUI) similar to a standard online card payment form including fields for card type, name, card number, card verification code (cvc) and expiry date. Then, user behaviour biometrics commonly known as keystroke, mouse and touchscreen (KMT) dynamics were captured while users entered fictitious card information on the GUI application. To capture such data, the Kivy Python library was used. The library contains event listeners capable of monitoring any occurrence of events such as key press, key release, mouse movement, mouse press or mouse release.
This data containing raw KMT data about each event was stored for further processing.

To illustrate how this data can be useful for user identification, we extracted features from the raw data and applied a simple machine learning classification task to generate results. Both datasets (i.e., \`raw\_kmt\_dataset' and \`feature\_kmt\_dataset') are included in this submission with a Jupyter notebook file (kmt\_feature\_classification.ipynb) which contains code necessary for performing the classification task.

## Institutions

- Edge Hill University


## Categories

Artificial Intelligence, Cybersecurity, Machine Learning, Biometrics, Authentication, Data Analytics Cybersecurity

## Related Links

Article

[https://www.researchgate.net/publication/361391929\_A\_behaviour\_biometrics\_dataset\_for\_user\_identification\_and\_authentication](https://www.researchgate.net/publication/361391929_A_behaviour_biometrics_dataset_for_user_identification_and_authentication)

cites this dataset

## Licence

CC BY 4.0Learn more

![plumX logo](https://cdn.plu.mx/3ba727faf225e19d2c759f6ebffc511d/plumx-inverse-logo.png)

Usage

- Views: 3634
- Downloads: 1064

![plumX logo](https://cdn.plu.mx/3ba727faf225e19d2c759f6ebffc511d/plumx-logo.png)

[View details](https://plu.mx/plum/a/?doi=10.17632/fnf8b85kr6&theme=plum-bigben-theme "PlumX Metrics Detail Page")

## Latest version

Version 1

Published:20 Jun 2022

DOI: 10.17632/fnf8b85kr6.1

Cite this dataset

Copy to clipboard

- [Home](https://data.mendeley.com/)
- \|
- [About](https://data.mendeley.com/about)
- \|
- [Accessibility Statement](https://data.mendeley.com/accessibility)
- \|
- [Archive Policy](https://data.mendeley.com/archive-process)
- \|
- [File Formats](https://data.mendeley.com/file-formats)
- \|
- [API Docs](https://data.mendeley.com/api/docs)
- \|
- [OAI](https://data.mendeley.com/oai?verb=Identify)
- \|
- [Mission](https://data.mendeley.com/mission)
- \|
- [Status Updates](https://digitalcommons.elsevier.com/status)

- [Terms of Use](https://www.mendeley.com/terms/)
- \|
- [Privacy Policy](https://www.elsevier.com/legal/privacy-policy)
- \|
- Cookie Settings

- All content on this site: Copyright © 2026 Elsevier inc, its licensors, and contributors. All rights are reserved, including those for text and data mining, AI training and similar technologies. For all open access content, the Creative Commons licensing terms apply.