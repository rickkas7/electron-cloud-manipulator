//1. Install dependencies
//npm install

//2. Run the program:
//npm start

const dgram = require('dgram');

const config = require('./config');

const argv = require('yargs').argv

const vorpal = require('vorpal')();

var modes = {
	data:true,
	latency:0,
	loss:0
};

const server = dgram.createSocket('udp4');


var dsAddr = config.get('DS_ADDR');
var dsPort = config.get('DS_PORT');
console.log('using device service address ' + dsAddr);


vorpal
	.command('data [action]', 'Turns data transmissions, both upload and download. Action = [on|off] or omit to toggle.')
	.action(function(args, callback) {
		handleAction(args.action, 'data');
		this.log('data ' + (modes.data ? 'on' : 'off'));
		
		callback();			
	});

vorpal
	.command('disconnect', 'Disconnect cloud connections.')
	.action(function(args, callback) {
		if (connections.length > 0) {
			this.log('disconnecting ' + connections.length + ' connections');
			while(connections.length > 0) {
				var conn = connections.pop();
				conn.client.destroy();
				conn.conn.destroy();
			}
		}
		else {
			this.log('disconnect - no connections');			
		}
		callback();			
	});

vorpal
	.command('latency [ms]', 'Simulate a high-latency network like satellite.')
	.action(function(args, callback) {
		if (args.ms) {
			modes.latency = parseInt(args.ms);
		}
		else {
			modes.latency = 0;
		}
		this.log('latency ' + modes.latency + ' ms');
		callback();			
	});

vorpal
	.command('loss [pct]', 'Randomly lose pct percent of packets (0 <= pct <= 100)')
	.action(function(args, callback) {
		if (args.pct) {
			modes.loss = parseInt(args.pct);
			if (modes.loss < 0) {
				modes.loss = 0;
			}
			if (modes.loss > 100) {
				modes.loss = 100;
			}
		}
		else {
			modes.loss = 0;
		}
		this.log('loss ' + modes.loss + '%');
		callback();			
	});


vorpal
	.delimiter('$')
	.show();

class Device {
	constructor(addr, port) {
		this.addr = addr;
		this.port = port;
		
		console.log(`new device ${addr}:${port}`);
		
		this.socket = dgram.createSocket('udp4');
		
		this.socket.on('error', (err) => {
			console.log(`from cloud error:\n${err.stack}`);
			server.close();
		});

		this.socket.on('message', (msg, rinfo) => {
			// console.log(`from cloud: ${msg} from ${rinfo.address}:${rinfo.port}`);
			
			if (modes.data && !losePacket()) {
				if (modes.latency == 0) {
					console.log('< ' + msg.length);
					server.send(msg, this.port, this.addr);
				}
				else {
					var port = this.port;
					var addr = this.addr;
					
					console.log('< ' + msg.length + ' queued');
					setTimeout(function() {
						console.log('< ' + msg.length);
						server.send(msg, port, addr);					
					}, modes.latency)
				}
			}
			else {
				console.log('< ' + msg.length + ' discarded');				
			}
		});

		this.socket.on('listening', () => {
			const address = this.socket.address();
			console.log(`device specific cloud port listening ${address.address}:${address.port}`);
		});
		this.socket.bind();
	}
	
	send(msg) {
		// Send to the real UDP device service from our client-specific socket
		this.socket.send(msg, dsPort, dsAddr);
	}
}
var devices = [];

function getDeviceForDeviceAddrPort(addr, port) {
	for(var d of devices) {
		if (d.addr == addr && d.port == port) {
			return d;
		}
	}
	var d = new Device(addr, port);
	devices.push(d);
	return d;
}

function losePacket() {
	if (modes.loss > 0) {
		return (Math.random() * 100) < modes.loss;
	}
	else {
		return false;
	}
}

server.on('error', (err) => {
	console.log(`server error:\n${err.stack}`);
	server.close();
});

server.on('message', (msg, rinfo) => {
	// console.log(`server got: ${msg} from ${rinfo.address}:${rinfo.port}`);

	var d = getDeviceForDeviceAddrPort(rinfo.address, rinfo.port);
	if (modes.data && !losePacket()) {
		if (modes.latency == 0) {
			console.log('> ' + msg.length);
			d.send(msg);
		}
		else {
			console.log('> ' + msg.length + ' queued');
			setTimeout(function() {
				console.log('> ' + msg.length);
				d.send(msg);					
			}, modes.latency);	
		}
	}
	else {
		console.log('> ' + msg.length + ' discarded');
	}
});

server.on('listening', () => {
	const address = server.address();
	console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(dsPort);

function handleAction(arg, sel) {
	if (arg === 'on') {
		modes[sel] = true;
	}
	else
	if (arg === 'off') {
		modes[sel] = false;		
	}
	else {
		modes[sel] = !modes[sel];
	}
}
