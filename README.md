# Electron cloud manipulator

*Simple tool to simulate various cloud connection issues*

## What it does

This is a node.js server that normally forwards cloud data packets to the real cloud. But it also has the ability delay and discard packets.

It's an interactive command prompt application, so you start it up and type commands to change the behavior in real time.

## Set up your device

The first thing you need to do is change your device to point at the server. 

- Note the IP address of the computer you're running the node.js server on.
- Put the Photon in DFU mode (blinking yellow)
- Run the command:

```
particle keys server --protocol udp --host 65.19.178.42 --port 5684 ec.pub.der
```

The ec.pub.der file is the public server key of the real Particle cloud server. It's included in the electron-cloud-manipulator directory so you don't need to download it separately.

Replace 65.19.178.42 with the IP address of your server. Note that this must be a public IP address so the Electron can see it; it can't be an internal 192.168.x.x or 10.x.x.x address!


## Run it!

- Install [nodejs](https://nodejs.org/) if you haven't already done so. I recommend the LTS version. Node 6 or later is required.
- Download this repository.
- Install the dependencies

```
cd electron-cloud-manipulator
npm install
```

- Run the program:

```
npm start
```

It should output something like this when you start it and an Electron makes a connection:

```
$ npm start

> electron-cloud-manipulator@0.0.1 start /Users/rickk/Documents/src/electron-cloud-manipulator
> node electron-cloud-manipulator.js

using device service address 34.201.112.170
$ server listening 0.0.0.0:5684
new device 176.83.139.143:55901
> 46
device specific cloud port listening 0.0.0.0:46637
< 33
```

The numbers are the number of bytes transmitted. 

\> is device to cloud

\< is cloud to device


## Commands

Enter commands at the $ prompt. Note that completion is provided, so you can just type "la" then tab and it will fill in latency for you.

### data [on|off]

The data command turns on and off data in both directions (upload and download). Omitting on or off toggles the current state.

### exit

Exit the program. You can also use quit or Ctrl-C (twice).

### help

List commands.

### latency <ms>

Set latency in milliseconds, typically used to simulate networks with high latency.

Setting ms to 0 turns off high latency mode, the default mode. Setting it higher than about 750 milliseconds will likely make connections impossible, due to the way the CoAP timeouts are set.

### loss <pct>

Sets random loss mode, 0 to 100%. 0 = no loss, the default behavior.

This simulates a bad cellular connection. See also the data command.

## Electron Device Firmware

There's Electron device firmware in the firmware directory. The use of this is optional, however it's good for initial testing. It's a fully function Tinker so you can control it using the Particle app.

You can also connect using a bidirectional serial terminal program (screen, PuTTY, CoolTerm, etc.) and type commands at it.

Commands are case-sensitive and are terminated by Return:

```
con - connect to the Particle cloud
dis - disconnect from the Particle cloud
keep [value] - set the keepAlive value
pub - publish a test event
ses - end session (publish spark/device/session/end)
```


## Restore your device

To put the Electron back to normal cloud mode run the command with no additional options.

```
particle keys server
```


