[Go to Axis Documentation](https://help.axis.com/en-us)

​

Search

[About your device](https://help.axis.com/en-us/axis-s2216#about-your-device)

[Setup examples](https://help.axis.com/en-us/axis-s2216#setup-examples)

[Get started](https://help.axis.com/en-us/axis-s2216#get-started)

[Install your device](https://help.axis.com/en-us/axis-s2216#install-your-device)

[Configure your device](https://help.axis.com/en-us/axis-s2216#configure-your-device)

[Manage local Windows® user accounts](https://help.axis.com/en-us/axis-s2216#manage-windows-user-accounts)

[Manage AXIS Camera Station Pro user accounts](https://help.axis.com/en-us/axis-s2216#manage-axis-camera-station-user-accounts)

[Manage your device](https://help.axis.com/en-us/axis-s2216#manage-your-device)

[Manage the built-in switch](https://help.axis.com/en-us/axis-s2216#manage-the-built-in-switch)

[About the built-in switch](https://help.axis.com/en-us/axis-s2216#about-the-built-in-switch)

[Log in to the switch's management page](https://help.axis.com/en-us/axis-s2216#log-in-to-the-switchs-management-page)

[Overview](https://help.axis.com/en-us/axis-s2216#overview)

[Turn on and turn off PoE](https://help.axis.com/en-us/axis-s2216#turn-on-and-turn-off-poe)

[Lock and unlock ports](https://help.axis.com/en-us/axis-s2216#lock-and-unlock-ports)

[Power management](https://help.axis.com/en-us/axis-s2216#power-management)

[Settings](https://help.axis.com/en-us/axis-s2216#settings)

[Maintenance](https://help.axis.com/en-us/axis-s2216#maintenance)

[Log](https://help.axis.com/en-us/axis-s2216#log)

[Product overview](https://help.axis.com/en-us/axis-s2216#product-overview)

[Troubleshooting](https://help.axis.com/en-us/axis-s2216#troubleshooting)

[Need more help?](https://help.axis.com/en-us/axis-s2216#need-more-help)

## About your device

AXIS Camera Station S22 Appliance series is an all-in-one recording solution with integrated, manageable PoE switch designed to deliver reliable high-definition surveillance with up to 4K resolution. For quick and easy installation, the appliance series is preconfigured and preloaded with AXIS Camera Station video management software including licenses plus all necessary system software. AXIS Camera Station offers an intuitive user interface that allows users to take full advantage of Axis wide range of video surveillance cameras and other IP products. With the operating system stored on a solid-state drive (SSD) and 5-year hardware warranty, the appliance series provides a reliable surveillance solution.

An AXIS Camera Station S22 Appliance consists of two parts:

- Switch: Built-in Power over Ethernet (PoE) switch.

- Computer: Preloaded with all necessary software you need to create a surveillance solution including the video management software AXIS Camera Station.


- The switch and computer are two separate parts.

- The switch and computer communicate through the internal uplink connectors: one is located on the switch Power Circuit Board and the other is located on the computer motherboard.

- The server uplink U2 is an external network interface that can connect to an existing network.

- Devices connected to server uplink U2 can not communicate with devices connected to the switch directly.

- Devices connected to server uplink U2 can communicate with devices connected to the switch through AXIS Camera Station.


## Setup examples

### Setup in an independent surveillance network

You can create an independent surveillance network which has no interconnectivity to another external network. This setup is a basic plug and play installation. The built-in switch’s DHCP server is enabled by default. As soon as you plug the cameras into the PoE ports, the cameras will power on and obtain an IP address and be accessible via AXIS Camera Station.

|     |     |
| --- | --- |
| Difficulty level | Basic |
| Benefits | Dedicated surveillance network with no interconnectivity to another external network<br>Plug and play installation |
| Limitations | Bandwidth<br>PoE budget<br>No remote access |
| Actions needed | Change the default password for the built-in switch<br>Register the AXIS Camera Station license |
| Connectors used | PoE enabled network connectors, port 1-12<br>USB 2.0 connector x2 (for keyboard and mouse)<br>DisplayportTM or HDMI connector |
| Connectors NOT used | RJ45 switch uplink (U1)<br>SFP switch uplink (U1)<br>RJ45 server uplink (U2)<br>USB 3.0 connector (front side)<br>Audio line in (front side)<br>Audio line out (front side) |

### Setup in an existing network

You can create a surveillance network within an existing network. This means that the surveillance network is separated from the existing network.

Note

When you use an additional recorder, for example, the AXIS S30 Recorders, the appliance does not route network data from the surveillance network to the server network for recording. Make sure that the AXIS S30 Recorders are connected to the same network as the cameras.

|     |     |
| --- | --- |
| Difficulty level | Advanced |
| Benefits | Ability to use an AXIS Camera Station client to connect to S22 series over the network.<br>Network segregation |
| Limitations | May require you to follow corporate network polices |
| Actions needed | Change the default password for the built-in switch<br>Register the AXIS Camera Station license |
| Connectors used | PoE enabled network connectors, port 1-12<br>RJ45 server uplink (U2) for connection to network<br>(Optional) USB 2.0 connector x2 (for keyboard and mouse)<br>(Optional) DisplayportTM or HDMI connector |
| Connectors NOT used | RJ45 switch uplink (U1)<br>SFP switch uplink (U1)<br>USB 3.0 connector (front side)<br>Audio line in (front side)<br>Audio line out (front side) |

## Get started

The standard workflow to configure an AXIS Camera Station recorder is:

01. [Install your device](https://help.axis.com/en-us/axis-s2216#install-your-device)

02. Configure Windows®. We recommend to:

    - Update Windows® to the latest version. See [Update Windows®](https://help.axis.com/en-us/axis-s2216#update-windows)

    - Create a standard user account. See [Create a user account](https://help.axis.com/en-us/axis-s2216#create-a-user-account)
04. [First-time configuration](https://help.axis.com/en-us/axis-s2216#first-time-configuration)

05. Update AXIS Camera Station to the latest version.

    - If your system is online: open the AXIS Recorder Toolbox app and click **Update AXIS Camera Station**.

    - If your system is offline: go to [axis.com](https://www.axis.com/en-us/) and download the latest version.
07. [Configure AXIS Camera Station Pro](https://help.axis.com/en-us/axis-s2216#configure-axis-camera-station)

08. Register you AXIS Camera Station licenses.

    - [License a system online](https://help.axis.com/en-us/axis-s2216#license-a-system-with-internet-connection)

    - [License a system offline](https://help.axis.com/en-us/axis-s2216#license-a-system-without-internet-connection)
10. Connect your system to the AXIS Camera Station mobile viewing app. See [Configure AXIS Secure Remote Access](https://www.axis.com/en-us/products/online-manual/34074#t10123489)


## Install your device

![](https://help.axis.com/image/t10132034.png)![](https://help.axis.com/image/t10132035.png)![](https://help.axis.com/image/t10132036.png)![](https://help.axis.com/image/t10132037.png)![](https://help.axis.com/image/t10132038.png)![](https://help.axis.com/image/t10132039.png)![](https://help.axis.com/image/t10132040.png)![](https://help.axis.com/image/t10132041.png)![](https://help.axis.com/image/t10132042.png)![](https://help.axis.com/image/t10132602.png)

## Configure your device

### First-time configuration

[![](https://i.ytimg.com/vi/WzE6FY5RnW4/hqdefault.jpg)](https://www.youtube.com/embed/WzE6FY5RnW4?autoplay=1&start=0&rel=0&enablejsapi=1&origin=https://help.axis.com)

After you have configured Windows®, AXIS Recorder Toolbox is opened automatically and you are guided through the first-time configuration wizard. In this wizard, you can configure several basic and necessary settings before you manage your device in AXIS Recorder Toolbox.

Note

The settings are for the server. To change the switch’s settings, go to the switch’s management page. See [Settings](https://help.axis.com/en-us/axis-s2216#settings).

1. Change the computer name if you want and click **Next**.

2. Under **Date and time**, configure the following settings and click **Next**.

   - Select a time zone.

   - To set up an NTP server, select **NTP server** and enter the NTP server address.

   - To set manually, select **Manual** and select a date and time.
4. Under **Network settings**, configure the following settings and click **Next**.

   - **Use automatic IP settings (DHCP)** and **Use automatic DNS settings** are turned on by default.

   - If your device is connected to a network with a DHCP server, the assigned IP address, subnet mask, gateway, and preferred DNS are automatically displayed.

   - If your device is not connected to a network or there is no DHCP server available, enter the IP address, subnet mask, gateway, and preferred DNS manually depending on the network requirements.
6. Click **Finish**. If you have changed the computer name, AXIS Recorder Toolbox will prompt you to restart the device.


### Connect to a server

Using the AXIS Camera Station Pro client, you can connect to multiple servers or a single server installed on the local computer or somewhere else on the network. You can connect to AXIS Camera Station Pro servers in different ways:

Last used serversConnect to the servers used in the previous session.This computerConnect to the server installed on the same computer as the client.Remote serverSee [Connect to a remote server](https://help.axis.com/en-us/axis-s2216#connect-to-a-remote-server).

Note

When trying to connect to a server for the first time, the client checks the server certificate ID. To ensure that you’re connecting to the correct server, manually verify the certificate ID with the one displayed in AXIS Camera Station Pro Service Control.

|     |     |
| --- | --- |
| **Server list** | To connect to servers from a server list, select a one from the **Server list** drop-down menu. Click ![](https://help.axis.com/image/t10130064.png) to create or edit the server lists. |
| **Import server list** | To import a server list file exported from AXIS Camera Station, click **Import server list** and browse to an .msl file. |
| **Delete saved passwords** | To delete saved usernames and passwords all connected servers, click **Delete saved passwords**. |
| **Change client proxy settings** | You might need to change the client proxy settings to connect to a server, click **Change client proxy settings**. |

#### Connect to a remote server

1. Select **Remote server**.

2. Select a server from the **Remote server** drop-down list or enter the IP or DNS address. If the server isn’t listed, click ![](https://help.axis.com/image/t10130089.png) to reload all the available remote servers. If the server is configured to accept clients on a different port than the default port number 55754, enter the IP address followed by the port number, for example, 192.168.0.5:46001.

3. You can:

   - Select **Log in as current user** to log in as the current Windows® user.

   - Clear **Log in as current user** and click **Log in**. Select **Other user** and provide another username and password to log in with different credentials.

#### Sign in to AXIS Secure Remote Access

Important

To improve security and functionality, we’re upgrading Axis Secure Remote Access (v1) to Axis Secure Remote Access v2. We’re discontinuing the current version on December 1st, 2025, and we strongly recommend that you upgrade to Axis Secure Remote Access v2 before that.

What does this mean for your AXIS Camera Station S22 Appliance Series system?

- After December 1st, 2025, you will no longer be able to remotely access your system using Axis Secure Remote Access (v1).
- To use Axis Secure Remote Access v2, you must upgrade to AXIS Camera Station Pro version 6.8. This upgrade is currently free for all AXIS Camera Station 5 users until March 1st, 2026.

Note

- When trying to connect to a server using Axis Secure Remote Access, the server can’t upgrade the client automatically.
- If the proxy server is between the network device and the AXIS Camera Station S22 Appliance Series server, you must configure the proxy settings in Windows on the AXIS Camera Station S22 Appliance Series server to access the server using AXIS Secure Remote Access.

1. Click the **Sign in to AXIS Secure Remote Access** link.

2. Enter your My Axis account credentials.

3. Click **Sign in**.

4. Click **Grant**.


### Configure AXIS Camera Station Pro

Before you start:

- Configure your network depending on your installation. See [Network configuration](https://help.axis.com/en-us/axis-s2216#network-configuration).

- Configure your server ports if needed. See [Server port configuration](https://help.axis.com/en-us/axis-s2216#server-port-configuration).

- Consider security issues. See [Security considerations](https://help.axis.com/en-us/axis-s2216#security-considerations).


After necessary configurations, you can start to work with AXIS Camera Station Pro:

1. [Start the video management system](https://help.axis.com/en-us/axis-s2216#start-axis-camera-station)

2. [Add devices](https://help.axis.com/en-us/axis-s2216#get-started-add-devices)

3. [Configure recording method](https://help.axis.com/en-us/axis-s2216#configure-recording-method)

4. [View live video](https://help.axis.com/en-us/axis-s2216#live-view-cameras)

5. [View recordings](https://help.axis.com/en-us/axis-s2216#get-started-playback-recordings)

6. [Add bookmarks](https://help.axis.com/en-us/axis-s2216#add-bookmarks)

7. [Export recordings](https://help.axis.com/en-us/axis-s2216#get-started-export-recordings)

8. [Play and verify recordings in AXIS File Player](https://help.axis.com/en-us/axis-s2216#play-and-verify-recordings-in-axis-file-player)


#### Start the video management system

Double-click the AXIS Camera Station Pro client icon to start the client. When you start the client for the first time, it attempts to log in to the AXIS Camera Station Pro server installed on the same computer as the client.

You can connect to multiple AXIS Camera Station Pro servers in different ways.

#### Add devices

The Add devices page opens the first time you start AXIS Camera Station Pro. AXIS Camera Station Pro searches the network for connected devices and shows a list of devices found.

1. Select the devices you want to add from the list. If you can’t find a device, click **Manual search** or type to filter.

2. Click **Add**.

3. To skip configuration and continue later with the added devices, click Close. To continue with the configuration:

1. Choose **Quick configuration** or **Site Designer configuration**. See for more information.

2. Select Add image overlay for date, time, and text if you’d like to add an overlay for the Axis cameras that don’t already have overlays configured.

3. Click **Next**.

4. Select your Recording method preference.

5. Click Next.

6. Choose your Retention time and Recording storage, and click Finish to apply the configuration.

#### Configure recording method

1. Go to **Configuration > Recording and events > Recording method**.

2. Select a camera.

3. Turn on **Motion detection**, or **Continuous**, or both.

4. Click **Apply**.


#### View live video

1. Open a **Live view** tab.

2. Select a camera to view its live video.


#### View recordings

1. Open a **Recordings** tab.

2. Select the camera you want to view recordings from.


#### Add bookmarks

1. Go to the recording.

2. In the timeline of the camera, zoom in and out and move the timeline to put the marker at your desired position.

3. Click ![](https://help.axis.com/image/t10130063.png).

4. Enter the bookmark name and description. Use keywords in the description to make the bookmark easy to find and recognize.

5. Select **Prevent recording deletion** to lock the recording.

6. Note





It’s not possible to delete a locked recording. To unlock the recording, clear the option or delete the bookmark.

7. Click **OK** to save the bookmark.


#### Export recordings

1. Open a **Recordings** tab.

2. Select the camera you want to export recordings from.

3. Click ![](https://help.axis.com/image/t10130069.png) to display the selection markers.

4. Drag the markers to include the recordings that you want to export.

5. Click ![](https://help.axis.com/image/t10130068.png) to open the **Export** tab.

6. Click **Export...**.


#### Play and verify recordings in AXIS File Player

1. Go to the folder with the exported recordings.

2. Double-click AXIS File Player.

3. Click ![](https://help.axis.com/image/t10130062.png) to show the recording’s notes.

4. To verify the digital signature:

1. Go to **Tools > Verify digital signature**.

2. Select **Validate with password** and enter your password.

3. Click **Verify**. The verification result page appears.
6. Note





   - Digital signature is different from Signed video. Signed video allows you to trace video back to the camera it came from, making it possible to verify that the recording wasn't tampered with. See [Signed video](https://www.axis.com/en-us/solutions/built-in-cybersecurity-features) and the camera’s user manual for more information.
   - If stored files don’t have any connection with an AXIS Camera Station database (non-indexed files), you need to convert them to make them playable in AXIS File Player. Contact Axis Technical support for help converting your files.

#### Network configuration

Configure proxy or firewall settings before using AXIS Camera Station Pro if the AXIS Camera Station Pro client, AXIS Camera Station Pro server, and the connected network devices are on different networks.

Client proxy settings

If a proxy server is between the client and the server, you must configure the proxy settings in Windows on the client computer. Contact Axis support for more information.

Server proxy settings

If the proxy server is between the network device and the server, you must configure the proxy settings in Windows on the server. Contact Axis support for more information.

NAT and Firewall

When a NAT, firewall, or similar separates the client and the server, configure the NAT or firewall to ensure that the HTTP port, TCP port, and streaming port specified in AXIS Camera Station Service Control can pass through the firewall or NAT. Contact the network administrator for instructions on configuring the NAT or firewall.

#### Server port configuration

AXIS Camera Station Pro server uses ports 55752 (HTTP), 55754 (TCP), 55756 (mobile communication), and 55757 (mobile streaming) for communication between the server and the client. You can change the ports in AXIS Camera Station Service Control if required.

#### Security considerations

To prevent unauthorized access to cameras and recordings, keep the following in mind:

- Use strong passwords for all network devices (cameras, video encoders, and auxiliary devices).

- Install AXIS Camera Station S22 Appliance Series server, cameras, video encoders, and auxiliary devices on a secure network separate from the office network. You can install the AXIS Camera Station S22 Appliance Series client on a computer on another network, for example, a network with internet access.

- Make sure all users have strong passwords. Windows® Active Directory provides a high level of security.


### License a system online

Both the AXIS Camera Station client and the server must have internet connection.

1. Go to **Configuration > Licenses > Management**.

2. Make sure **Manage licenses online** turns on.

3. Sign in with your My Axis account.

4. The license key automatically generates and appears under **License keys**.

5. If you purchased license keys separately, enter your license key under **Add license key**.

6. Click **Add**.

7. In AXIS Camera Station client, make sure your license keys appear under **Configuration > Licenses > Keys**.


### License a system offline

01. Go to **Configuration > Licenses > Management**.

02. Turn off **Manage licenses online**.

03. Click **Export system file**.

04. Save your system file to a USB flash drive.

05. Sign in with your My Axis account.

06. Click **Upload system file** to upload the system file from your USB flash drive.

07. The license key automatically generates and shows under **License keys**.

08. If you purchased license keys separately, enter your license key under **Add license key**.

09. Click **Add**.

10. Under **License keys**, click **Download license file** and save the file to a USB flash drive.

11. In AXIS Camera Station client, go to **Configuration > Licenses > Management**.

12. Click **Import license file** and select the license file on your USB flash drive.

13. Make sure your license keys appear under **Configuration > Licenses > Keys**.


## Manage local Windows® user accounts

### Create a user account

To help keep your personal data and information more secure, we recommend that you add a password for each local account.

Important

Once you create a password for a local account, don't forget it. There’s no way to recover a lost password for local accounts.

1. Go to **Settings > Accounts > Other users > Add other user** and click **Add account**.

2. Click **I don’t have this person’s sign-in information**.

3. Click **Add a user without a Microsoft account**.

4. Enter a user name, password and password hint.

5. Click **Next** and follow the instructions.


### Create an administrator account

1. Go to **Settings > Accounts > Other people**.

2. Go to the account you want to change and click **Change account type**.

3. Go to **Account type** and select **Administrator**.

4. Click **OK**.

5. Restart your device and sign in with the new administrator account.


### Create a local user group

1. Go to **Computer Management**.

2. Go to **Local Users and Groups > Group**.

3. Right-click **Group** and select **New Group**.

4. Enter a group name and a description.

5. Add group members:

1. Click **Add**.

2. Click **Advanced**.

3. Find the user account(s) you want to add to the group and click **OK**.

4. Click **OK** again.
7. Click **Create**.


### Delete a user account

Important

When you delete an account you remove the user account from the login screen. You also remove all files, settings and program data stored on the user account.

1. Go to **Settings > Accounts > Other people**.

2. Go to the account you want to remove and click **Remove**.


### Change a user account's password

1. Log in with an administrator account.

2. Go to **User Accounts > User Accounts > Manage another account in sequence**.

You’ll see a list with all user accounts on the device.

3. Select the user account whose password you would like to change.

4. Click **Change the password**.

5. Enter the new password and click **Change password**.


### Create a password reset disk for a user account

We recommend to create a password reset disk on a USB flash drive. With this, you can reset the password. Without a password reset disk, you can’t reset the password.

Note

If you’re using Windows® 10, or later, you can add security questions to your local account in case you forget your password, so you don't need to create a password reset disk. To do this, got to **Start** and click **Settings > Sign-in options > Update your security questions**.

1. Sign in to your device with a local user account. You can’t create a password reset disk for a connected account.

2. Plug an empty USB flash drive into your device.

3. From the Windows® search field, go to ﻿ **Create a password reset disk**.

4. In the **Forgotten Password** setup assistant, click **Next**.

5. Select your USB flash drive and click **Next**.

6. Type your current password and click **Next**.

7. Follow the onscreen instructions.

8. Remove the USB flash drive and keep it in a safe placet. You don't have to create a new disk when you change your password even if you change it several times.


## Manage AXIS Camera Station Pro user accounts

### User permissions

Go to **Configuration > Security > User permissions** to view the users and groups that exists in AXIS Camera Station S22 Appliance Series.

Note

Administrators of the computer that runs AXIS Camera Station S22 Appliance Series server are automatically given administrator privileges to AXIS Camera Station S22 Appliance Series. You can’t change or remove the Administrators group's privileges.

Before you can add a user or group, register the user or group on the local computer or make sure they have an Windows® Active Directory user account. To add users or groups, see [Add users or groups](https://help.axis.com/en-us/axis-s2216#add-users-or-groups).

When a user is part of a group, the user gets the highest role permission assigned to the individual or the group. The user also gets the access granted as an individual and receives the rights as part of a group. For example, a user has access to camera X as an individual. The user is also a member of a group that has access to cameras Y and Z. The user therefore has access to cameras X, Y, and Z.

|     |     |
| --- | --- |
|  | Indicates the entry is a single user. |
|  | Indicates the entry is a group. |
| **Name** | Username as it appears in the local computer or Active Directory. |
| **Domain** | The domain that the user or group belongs to. |
| **Role** | The access role given to the user or group. <br>Possible values: Administrator, Operator, and Viewer. |
| **Details** | Detailed user information as it appears in the local computer or Active Directory. |
| **Server** | The server that the user or group belongs to. |

### Add users or groups

Microsoft Windows® and Active Directory users and groups can access AXIS Camera Station S22 Appliance Series. To add a user to AXIS Camera Station S22 Appliance Series, you must add users or a group to Windows®.

To add a user in Windows® 10 and 11:

- Press the Windows key + X and select **Computer Management**.

- In the **Computer Management** window, navigate to **Local Users and Groups** > **Users**.

- Right-click on **Users** and select **New User**.

- In the popup dialog, enter the new user’s details and uncheck **User must change password at next login**.

- Click **Create**.


If you use an Active Directory domain, consult your network administrator.

Add users or groups

1. Go to **Configuration > Security > User permissions**.

2. Click **Add**.

You can see the available users and groups in the list.

3. Under **Scope**, select where to search for users and groups.

4. Under **Show**, select to show users or groups.

The search result doesn’t display if there are too many users or groups. Use the filter function.

5. Select the users or groups and click **Add**.


| Scope |
| --- |
| **Server** | Select to search for users or groups on the local computer. |
| **Domain** | Select to search for Active Directory users or groups. |
| **Selected server** | When connected to multiple AXIS Camera Station S22 Appliance Series servers, select a server from the **Selected server** drop-down menu. |

Configure a user or group

1. Select a user or group in the list.

2. Under **Role**, select **Administrator**, **Operator**, or **Viewer**.

3. If you selected **Operator** or **Viewer**, you can configure the user or group privileges. See [User or group privileges](https://help.axis.com/en-us/axis-s2216#user-or-group-privileges).

4. Click **Save**.


Remove a user or group

1. Select the user or group.

2. Click **Remove**.

3. In the pop-up dialog, click **OK** to remove the user or group.


### User or group privileges

There are three roles you can give to a user or group. For how to define the role for a user or group, see [Add users or groups](https://help.axis.com/en-us/axis-s2216#add-users-or-groups).

AdministratorFull access to the entire system, including access to live and recorded video of all cameras, all I/O ports, and views. This role is required to configure anything in the system.OperatorSelect cameras, views, and I/O ports to get access to live and recorded. An operator has full access to all functionality of AXIS Camera Station S22 Appliance Series except system configuration.ViewerAccess to live video of selected cameras, I/O ports, and views. A viewer doesn’t have access to recorded video or system configuration.

Cameras

The following access privileges are available for users or groups with the **Operator** or **Viewer** role.

|     |     |
| --- | --- |
| **Access** | Allow access to the camera and all camera features. |
| **Video** | Allow access to live video from the camera. |
| **Audio listen** | Allow access to listen from the camera. |
| **Audio speak** | Allow access to speak to the camera. |
| **Manual Recording** | Allow to start and stop recordings manually. |
| **Mechanical PTZ** | Allow access to mechanical PTZ controls. Only available for cameras with mechanical PTZ. |
| **PTZ priority** | Set the PTZ priority. A lower number means a higher priority. No assigned priority is set to `0`. An administrator has the highest priority. When a role with higher priority operates a PTZ camera, others can’t operate the same camera for 10 seconds by default. Only available for cameras with mechanical PTZ and have **Mechanical PTZ** selected. |

Views

The following access privileges are available for users or groups with the **Operator** or **Viewer** role. You can select multiple views and set the access privileges.

|     |     |
| --- | --- |
| **Access** | Allow access to the views in AXIS Camera Station S22 Appliance Series. |
| **Edit** | Allow to edit the views in AXIS Camera Station S22 Appliance Series. |

I/O

The following access privileges are available for users or groups with the **Operator** or **Viewer** role.

|     |     |
| --- | --- |
| **Access** | Allow full access to the I/O port. |
| **Read** | Allow to view the state of the I/O port. The user can’t change the port state. |
| **Write** | Allow to change the state of the I/O port. |

System

You can’t configure grayed out access privileges in the list. A check mark means the user or group has this privilege by default.

The following access privileges are available for users or groups with the **Operator** role. **Take snapshots** is also available for the **Viewer** role.

|     |     |
| --- | --- |
| **Take snapshots** | Allow to take snapshots in the live view and recording mode. |
| **Export recordings** | Allow exporting recordings. |
| **Generate incident report** | Allow generating incident reports. |
| **Prevent access to recordings older than** | Prevent access to recordings older than the specified number of minutes. Users can’t find these recordings when they search. |
| **Access alarms, tasks, and logs** | Get alarm notifications and allow access to the **Alarms and tasks** bar and **Logs** tab. |
| **Access data search** | Allow searching for data to track what happened at the time of an event. |
| **Add categories to events** | Allow adding categories to events in the Recordings tab. |
| **Remove categories from event** | Allow removing categories from events in the Recordings tab. |

Access control

The following access privileges are available for users or groups with the **Operator** role. **Access Management** is also available for the **Viewer** role.

|     |     |
| --- | --- |
| **Access control configuration** | Allow configuration of doors and zones, identification profiles, card formats and PIN, encrypted communication, and multi-server. |
| **Access management** | Allow access management and access to the active directory settings. |

The following access privileges are available for users or groups with the **Viewer** role.

System health monitoring

The following access privileges are available for users or groups with the **Operator** role. **Access to system health monitoring** is also available for the **Viewer** role.

|     |     |
| --- | --- |
| **Configuration of system health monitoring** | Allow configuration of the system health monitoring system. |
| **Access to system health monitoring** | Allow access to the system health monitoring system. |

## Manage your device

### Update Windows®

Windows® periodically checks for updates. When an update is available, your device automatically downloads the update but you've to install it manually.

Note

Recording will be interrupted during a scheduled system restart.

To manually check for updates:

1. Go to **Settings > Windows Update**.

2. Click **Check for updates**.


### Configure Windows® update settings

It is possible to change how and when Windows® do its updates to suit your needs.

Note

All ongoing recordings stop during a scheduled system restart.

1. Open the Run app.

   - Go to **Windows System > Run**, or
3. Type `gpedit.msc` and click **OK**. The Local Group Policy Editor opens.

4. Go to **Computer Configuration > Administrative Templates > Windows Components > Windows Update**.

5. Configure the settings as required, see example.


#### Example

> To automatically download and install updates without any user interaction and have the device restart, if necessary, at out of office hours, use the following configuration:
>
> 1. Open **Always automatically restart at the scheduled time** and select:
>
>    1. **Enabled**
>
>    2. **The restart timer will give users this much time to save their work (minutes)**: 15.
>
>    3. Click **OK**.
> 3. Open **Configure Automatic Updates** and select:
>
>    1. **Enabled**
>
>    2. **Configure Automatic updates**: Auto download and schedule the install
>
>    3. **Schedule Install day**: Every Sunday
>
>    4. **Schedule Install time**: 00:00
>
>    5. Click **OK**.
> 5. Open **Allow Automatic Updates immediate installation** and select:
>
>    1. **Enabled**
>
>    2. Click **OK**.

### Reset your server

You can use the server reset button to reset your server. It will take more than one hour to reset your server.

1. Power off your device.

2. Press and hold the server reset button for 5 seconds. Windows® RE will be started.

3. Select **Troubleshoot**.

4. Select **Reset your PC**.

5. Select **Keep my files** or **Remove everything**. If you select **Keep my files**, you need to provide the administrator credentials.

6. Follow the instructions on the screen.

7. The server reboots and starts the procedure to restore Windows® to factory default settings.


[![](https://i.ytimg.com/vi/EZtUEpGsB6g/hqdefault.jpg)](https://www.youtube.com/embed/EZtUEpGsB6g?autoplay=1&start=0&rel=0&enablejsapi=1&origin=https://help.axis.com)

Reset your server to factory default settings

### Reset your switch

You can reset your switch to factory default settings by using one of the following ways:

- The switch reset button. Before you start, ensure that the switch is on. Press and hold the switch reset button for about 5 seconds until the power button LED at the rear side of the device turns blue. It will take about 6 minutes.

- The switch’s management page. See [Reset to factory default settings](https://help.axis.com/en-us/axis-s2216#reset-to-factory-default-settings).


### Add additional storage

The demand for storage can differ. Retention time of stored data or for storing high-resolution recordings often results in a need to install more storage. This section explains how to install an additional hard drive in your AXIS S22 series product.

Note

Follow the instructions below to add additional storage to applicable AXIS S22 products. These instructions are as-is, and Axis Communications AB takes no responsibility for loss of data or misconfiguration during these steps. The standard precautions should be taken to backup data that is business critical. The following procedure of expanding storage will not be supported by Axis Technical Support.

Note

To avoid electrostatic discharge, we recommend that you always use a static mat and static strap while working on internal system components.

Warranty

Detailed information about warranty is available at: [www.axis.com/support/warranty](https://www.axis.com/en-us/support/warranty).

[![](https://i.ytimg.com/vi/W1cdKK4HOTk/hqdefault.jpg)](https://www.youtube.com/embed/W1cdKK4HOTk?autoplay=1&start=0&rel=0&enablejsapi=1&origin=https://help.axis.com)

## Manage the built-in switch

### About the built-in switch

The AXIS Camera Station S22 Appliance Series comes with an integrated Power over Ethernet (PoE) switch. You can configure and manage the built-in switch.

The purpose of the switch is to segregate traffic on the network so that security cameras and related traffic managed by the switch (PoE ports and U1 network uplinks) are not shared with other networks.

- The switch’s power management follow these rules:
- Each port can reserve power according to the connected powered device’s PoE class. Power can also be allocated manually and by actual consumption.

- If the actual power consumption for a given port exceeds the reserved power for that port, it will shut down.

- Ports will shut down when the actual power consumption for all ports exceeds the total amount of power that the power supply can deliver. The ports are then shut down according to the ports priority where a lower port number means higher priority.


Select a language

In the switch’s management page, go to ![](https://help.axis.com/image/t10130155.png)**\> Language** and select the language that you want to use.

Go to the help pages

In the switch’s management page, go to ![](https://help.axis.com/image/t10130155.png)**\> Help**.

### Log in to the switch's management page

1. Go to the switch’s management page.

   - From the web browser, enter the switch’s IP address. By default: 192.168.0.1

   - From AXIS Recorder Toolbox, go to **Switch > Open the switch configuration**.

   - From AXIS Camera Station Client, go to **Configuration > Switch > Management**. See [Configure switch](https://www.axis.com/en-us/products/online-manual/34074#t10137142).
3. Log in with your credentials.

   - user name: `admin`

   - password: you can find the randomly generated password on a sticker underneath the product or a self-adhesive sticker in the box content.

To log out the switch's management page, go to ![](https://help.axis.com/image/t10130155.png)**\> Log out**.

### Overview

In the switch’s management page, click **Overview**.

General information

|     |     |     |
| --- | --- | --- |
| Category | Item | Description |
| Ports summary | Active ports | The number of ports that are in use. |
| Ports using PoE | The number of ports with PoE enabled that are in use. |
| Locked ports | The number of ports that are locked. |
| Current PoE usage |  | The total power in watts and percentage consumed by the PoE devices. |
| Port status |  | Click the port to see the details about the port. |
| Active | The port is in use. |
| Inactive | The port is ready to be used. |
| Blocked | The port is blocked. |

Port list

|     |     |
| --- | --- |
| Item | Description |
| Port | The port number that the device is connected to. |
| Device | The name of the device connected to the port. |
| IP address | The IP address of the device connected to the port. |
| PoE | Displays the PoE status. You can click the icon to turn on or turn off PoE on the port. |
| Power consumption (W) | The power in watts consumed by the device connected to the port. |
| Upstream (Mbit/s) | The average data rate in megabits per second for outbound data on the port. |
| Downstream (Mbit/s) | The average data rate in megabits per second for inbound data on the port. |
| MAC address | The MAC address of the device connected to the port. |
| Lock | Displays whether the port is locked. You can click the icon to lock or unlock the port. |

#### Turn on and turn off PoE

Turn on PoE on a port

1. In the switch’s management page, click **Overview**.

2. In the PoE column, click ![](https://help.axis.com/image/t10141123.png) to turn on PoE on the specific port.


Turn on PoE on all ports

1. In the switch’s management page, click **Overview**.

2. Click ![](https://help.axis.com/image/t10141121.png) and select **Turn on PoE on all ports**.


Turn off PoE on a port

1. In the switch’s management page, click **Overview**.

2. In the PoE column, click ![](https://help.axis.com/image/t10141124.png) to turn off PoE on the specific port.


Turn off PoE on all ports

1. In the switch’s management page, click **Overview**.

2. Click ![](https://help.axis.com/image/t10141121.png) and select **Turn off PoE on all ports**.


Note

You can also turn on and turn off PoE in the Power management page.

#### Lock and unlock ports

You can lock a MAC address to a port so that only traffic coming from that MAC address will pass. This improves security and prevents unauthorized users from connecting a laptop or other devices to the security network.

Lock a port

1. In the switch’s management page, click **Overview**.

2. In the Lock column, click ![](https://help.axis.com/image/t10141123.png) to lock the port.


Lock all ports

1. In the switch’s management page, click **Overview**.

2. Click ![](https://help.axis.com/image/t10141121.png) and select **Lock all ports**.


Unlock a port

1. In the switch’s management page, click **Overview**.

2. In the Lock column, click ![](https://help.axis.com/image/t10141124.png) to unlock the port.


Unlock all ports

1. In the switch’s management page, click **Overview**.

2. Click ![](https://help.axis.com/image/t10141121.png) and select **Unlock all ports**.


### Power management

In the switch’s management page, click **Power management**.

General information

- Ports using PoE: the number of ports with PoE enabled that are in use.

- Current PoE usage: the PoE power in watts consumed by devices and the percentage of used PoE power out of the total dedicated PoE power.

- Power requested: the total power in watts and percentage allocated to devices. Only available when you select **Reserved power** as the power allocation method.


Port list

|     |     |
| --- | --- |
| Item | Description |
| Port | The port number that the device is connected to. |
| PoE | The PoE status. You can click the icon to turn on or turn off PoE on the port. |
| PoE class | The PoE class of the connected device. |
| Priority | The priority of power allocation to the connected device. Lower numbered ports have higher priority. By default, the priority is the port number. |
| Power consumption (W) | The power in watts consumed by the device connected to the port. |
| Power requested (W) | The power in watts requested by the device connected to the port.<br>Only available when you select **Reserved power** as the power allocation method. |
| Power allocated (W) | Manually adjust the power allocated to the port. <br>Only available when you select **Reserved power > Manual** as the power allocation method. |

#### Allocate power

PoE power can be allocated to the connected devices in the following ways:

- Reserved power: In this method, each port reserves the amount of power. The total reserved power cannot exceed the total power budget. A port will not be powered up if the device tries to reserve more power than available. This method ensures that the connected devices will be powered.

  - PoE class: Each port automatically determines the amount of power to reserve according to the PoE class of the connected device.

  - Manual: You can manually adjust the amount of power allocated to each port by changing the value under **Power allocated**.

  - LLDP-MED: Each port determines the amount of power to reserve by exchanging PoE information using the LLDP protocol.

  - Note





    The power allocation method LLDP-MED works only for cameras with firmware 9.20 or later.
- Actual consumption: In this method, devices use the amount of power they need until reaching the power budget. A port will be shut down if the device consumes more power than available. The ports are shut down according to the priority.


To change the power allocation method:

1. In the switch’s power management page, go to **Power management**.

2. To use the reserved power method, select **Reserved power** under **Allocate power** and select **PoE Class**, **Manual**, or **LLDP-MED** under **Reserved power determined by**.

3. To use the actual consumption method, select **Actual consumption** under **Allocate power**.

4. If you have selected **Manual**, go to the Power allocated column and change the power allocated to the connected device.

5. If you want to change the priority of the connected device, select a priority for that device. The priority of other devices will change automatically.


#### Example

> In this example, the switch has a total power budget of 135 W. A PoE class 4 device requests 30 W power and actually consumes 15 W power. A PoE class 2 device requests 7 W power and actually consumes 5 W power.
>
> Allocate power by PoE class
>
> - Each port reserves the amount of power according to the device’s PoE class.
>
> - The switch can power 4 PoE class 4 devices and 2 PoE class 2 devices.
>
> - The total power reserved is (4 x 30) + (2 x 7) =134 W.
>
> - The actual power consumed is (4 x 15) + (2 x 5) = 70 W.
>
> - In this way, all connected devices are guaranteed enough power and the priority is less important.
>
>
> Allocate power manually
>
> - The reserved power is manually adjusted to 20 W for PoE class 4 devices.
>
> - The switch can power 5 PoE class 4 devices and 3 PoE class 2 devices.
>
> - The total power reserved is (5 x 20) + (3 x 7) = 121 W.
>
> - The actual power consumed is (5 x 15) + (3 x 5) = 90 W.
>
> - In this way, all connected devices are guaranteed enough power and the priority is less important.
>
>
> Allocate power by actual consumption
>
> - When connecting a device, the switch will power it up if the max power limit can be ensured at that time.
>
> - First the switch connects to five PoE class 4 devices and the actual consumption of power is 5 x 15 = 75 W.
>
> - Later each device consumes up to 30 W power. The switch cannot power five devices and the port 5 with the lowest priority is shut down. The actual consumption is 4 x 30 = 120 W.
>
> - In this way, the priority of the port is important. The port with the lowest priority will be shut down first when the actual consumption exceeds the power budget.

### Settings

#### Configure network settings

You can change the switch’s IP address. But for most camera installations, we recommend using the default settings. The reason for this is that a surveillance network is normally isolated from other networks, for example a corporate LAN. In this case, you would only use the surveillance network to manage and collect surveillance devices and data from the video management software installed on the server.

1. In the switch’s management page, go to **Settings > Network settings**.

2. Enter the connection type, IP address, subnet mask, gateway, DNS1, DNS2, and hostname.

3. Note





The factory default settings are: a static IP connection with address 192.168.0.1 and a subnet mask with address 255.255.255.0.

4. Click **Save**.


#### Configure date and time

1. In the switch’s management page, go to **Settings > Date and time**.

2. Select the country and time zone.

3. To set the time manually, select **Manual** and manually adjust the time.

4. To set up an NTP server, select **NTP server** and enter the NTP server address.

5. Note





NTP only works when the switch is connected to a network and configured with Internet access.

6. Click **Save**.


#### Configure DHCP server

Important

If the switch’s DHCP server is enabled and your AXIS Camera Station S22 Appliance is connected to an external network with its own DHCP server, you will have IP address conflicts. This might result in the corporate network not working.

You can configure the switch to use its internal DHCP server for assigning IP addresses to connected devices. When you use the switch uplink connection to allow devices to access or being accessed by external application, you must specify the gateway and DNS addresses.

1. In the switch’s management page, go to **Settings > DHCP server**.

2. Select **Use DHCP server**.

3. Enter the start IP address, end IP address, subnet mask, gateway, DNS 1, DNS 2, lease length, and domain name.

4. Click **Save**.


#### Configure SNMP

1. In the switch’s management page, go to **Settings > SNMP**.

2. Enter the server name, contact, and location used for the SNMP connection.

3. If you want to use SNMPV1 or SNMPV2c, select **SNMPV1 / SNMPV2c** and enter the read community.

4. If you want to use SNMPV3, select **SNMPV3 (MD5)** and enter the username and password.

5. Note





Currently we only support MD5 authentication used for SNMP.

6. Click **Save**.


#### Configure web settings

1. In the switch’s management page, go to **Settings > Web settings**.

2. Enter the port number.

3. For more secure connection, select **Secure connection (HTTPS)** and enter the port number used for HTTPS connection.

4. Click **Save**.


Important

If you have changed the port number, ensure you record the new port number. If you’ve forgotten the new port number, contact Axis support.

### Maintenance

#### Update firmware

1. In the switch’s management page, go to **Maintenance > Update firmware**.

2. Drag and drop the firmware file or click **Browse** and navigate to the firmware file.

3. Click **Upload**.

4. After the firmware updating is done, reboot the switch.


#### Reboot the switch

Important

While the switch reboots, all connected devices will temporarily lose connection with the switch including PoE.

1. In the switch’s management page, go to **Maintenance > Reboot switch**.

2. Click **Reboot** and **Yes**.

3. When the switch reboots after a few minutes, enter your username and password to log in.


#### Backup the switch's settings

Note

The username and password are included in the backup file.

1. In the switch’s management page, go to **Maintenance > Backup and restore**.

2. Click **Create a backup file**. The backup file in the .bin format is created in the **Downloads** folder.


#### Restore the switch's settings

Note

To restore the switch’s settings, you must previously have created a backup file.

1. In the switch’s management page, go to **Maintenance > Backup and restore**.

2. Drag and drop the backup file or click **Browse** and navigate to the backup file.

3. Click **Upload**.


It could take a few minutes to restore the switch from the backup file. Once the settings are restored, the switch will automatically reboot and you need to log in again.

#### Manage certificates

1. In the switch’s management page, go to **Maintenance > Manage certificates**.

2. Click ![](https://help.axis.com/image/t10141122.png) and navigate to your private key file.

3. Click ![](https://help.axis.com/image/t10141122.png) and navigate to your certificate file.

4. Click ![](https://help.axis.com/image/t10141122.png) and navigate to your CA bundle file.

5. Click **Save**.

6. Reboot the switch.


#### Change password

You can change the switch's default password to a password you choose yourself.

Important

Ensure you select a password you remember. If you have forgotten the new password, contact Axis support.

1. In the switch’s management page, go to **Maintenance > Change password**.

2. Enter the current password and your new password as required.

3. Click **Save**.


#### Reset to factory default settings

1. In the switch’s management page, go to **Maintenance > Reset to factory default settings**.

2. Click **Reset** and **Yes**.


After the reset is done, the switch will reboot automatically.

### Log

In the switch’s management page, click **Log** to see a list of logs. Click **Refresh** to refresh the list. Click the column title to sort in alphabetical order.

|     |     |
| --- | --- |
| Item | Description |
| Time | The date and time when the log event happened. |
| Level | The serious level shown as warning icons. |
| User | The user that initiates the log event. |
| IP address | The IP address of the user that initiates the log event. |
| Service | The service that the log event is related to. Possible values are desktop, network, system, ntpd, storage, dhcp, etc. |
| Event | The description about the log event. |

#### Create switch reports

1. In the switch’s management page, click **Log**.

2. Click Create switch report.


## Product overview

### Front side

1. Audio in
2. Audio out
3. System power LED
4. Disk activity LED
5. Status LEDs of PoE ports
6. USB 3.0

### Rear side

01. Power connector
02. PoE ports
03. SFP switch uplink (U1)
04. RJ45 switch uplink (U1)
05. Switch reset button
06. Server reset button
07. USB 2.0 x2
08. RJ45 server uplink (U2)
09. HDMI port
10. Displayport
11. Power button

### Specifications

Front LEDs

|     |     |     |
| --- | --- | --- |
| LED indicator | Color | Indication |
| Power | Green | Server on |
| Network ports | Green | Link state |
| Hard drive | Blinks amber | Hard drive activity |
| Blinks red | Possible hard drive failure |

Rear LEDs

|     |     |     |
| --- | --- | --- |
| Network speed and activity | Color | Indication |
| Right LED | Amber | 10/100 Mbit/s |
| Green | 1000 Mbit/s |
| Left LED | Blinks green | Network activity |

|     |     |     |
| --- | --- | --- |
| Power button | Color | Indication |
| When the power button is not pressed | Off | Switch and server off |
| Blue | During switch boot |
| Green | Switch boot completed |
| When the power button is pressed | Red (flashes once for 1s) | Power on |
| >5s blinks green | Restarting server |
| >10s steady blue | Power off switch |
| >15s blinks blue | Power off switch and server |

## Troubleshooting

### Perform a system recovery

If the device has had a complete system failure, you must use a recovery image to recreate the Windows® system. To download the AXIS Network Video Recorder Recovery Kit, contact Axis technical support and supply the serial number of your device.

01. Download the AXIS Network Video Recorder Recovery Kit and AXIS Network Video Recorder Recovery: ISO to USB Tool.

02. Insert a USB drive into your computer.

    - Use a USB drive with a minimum of 16 GB.

    - The USB drive will be formatted, and all existing data will be erased.
04. Run the AXIS Network Video Recorder Recovery: ISO to USB Tool and follow the onscreen instructions.

    Writing data to the USB drive takes approximately 10 to 15 min. Don’t remove the USB drive until the process is complete.

05. After the AXIS Network Video Recorder Recovery: ISO to USB Tool is complete, take the USB drive and plug it into your device.

06. Start your device.

07. When you see the Axis splash screen, press F12.

08. Click **UEFI: USB Drive**.

09. Navigate to your USB drive and press enter. The system boots into the AXIS Network Video Recorder Recovery Kit.

10. Click **Reinstall Operating System**.

    The recovery takes roughly 10 to 15 min to complete. You find detailed instructions in the download for the recovery kit.


### Troubleshoot AXIS Camera Station Pro

For information about how to troubleshoot AXIS Camera Station Pro, go to the [AXIS Camera Station Pro user manual](https://help.axis.com/en-us/axis-camera-station-pro).

## Need more help?

### Useful links

- [AXIS Camera Station Pro user manual](https://help.axis.com/en-us/axis-camera-station-pro)

- [Sign in to AXIS Secure Remote Access](https://help.axis.com/en-us/axis-camera-station-pro#sign-in-to-axis-secure-remote-access)

- [Sign in to AXIS Secure Remote Access v2](https://help.axis.com/en-us/axis-camera-station-pro#sign-in-to-axis-secure-remote-access-v2)

- [What to include in an Antivirus allow list for AXIS Camera Station](https://www.axis.com/en-us/support/faq/FAQ116307)


### Contact support

If you need more help, go to [axis.com/support](https://www.axis.com/en-us/support).

Link copied to clipboard


Close


Could not find null help


Close


Could not find element on page


Close


Cookie settings

Axis uses cookies to remember your user preferences, for storing anonymized user statistics, for marketing, and to understand how people use our sites so that we can improve the quality of our services. We use cookies to track trends and patterns of how people use our sites.


More information: [Cookie policy](https://www.axis.com/privacy/cookie-policy) and [Cookie list](https://www.axis.com/privacy/cookie-list)

Your preferences

Necessary


Functional


Measurement


Marketing


You may change your preferences at any time through the cookie settings link at the bottom of each page.


Accept all

Only Necessary

Save preferences