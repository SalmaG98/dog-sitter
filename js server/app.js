var settings = {
	ip: "192.168.2.14",
	port: 3443
};

/**
 * Created by middleca on 7/18/15.
 */

//based on a sample from here
//	http://stackoverflow.com/questions/19548755/nodejs-write-binary-data-into-writablestream-with-buffer

var fs = require("fs");

var samplesLength = 1000;
var sampleRate = 8000;


var outStream = fs.createWriteStream("test.wav");


var writeHeader = function() {
	var b = new Buffer(1024);
	b.write('RIFF', 0);
	/* file length */
	b.writeUInt32LE(32 + samplesLength * 2, 4);
	//b.writeUint32LE(0, 4);

	b.write('WAVE', 8);
	/* format chunk identifier */
	b.write('fmt ', 12);

	/* format chunk length */
	b.writeUInt32LE(16, 16);

	/* sample format (raw) */
	b.writeUInt16LE(1, 20);

	/* channel count */
	b.writeUInt16LE(1, 22);

	/* sample rate */
	b.writeUInt32LE(sampleRate, 24);

	/* byte rate (sample rate * block align) */
	b.writeUInt32LE(sampleRate * 2, 28);

	/* block align (channel count * bytes per sample) */
	b.writeUInt16LE(2, 32);

	/* bits per sample */
	b.writeUInt16LE(16, 34);

	/* data chunk identifier */
	b.write('data', 36);

	/* data chunk length */
	//b.writeUInt32LE(40, samplesLength * 2);
	b.writeUInt32LE(0, 40);


	outStream.write(b.slice(0, 50));
};





writeHeader(outStream);





var net = require('net');
console.log("connecting...");
client = net.connect(settings.port, settings.ip, function () {
	client.setNoDelay(true);


    client.on ("drain",function(data){
        console.log("ending");
    });

    fs.watchFile ("test.wav", (curr, prev) => {
        console.log("file Changed");
      });

    client.on("data", function (data) {

        console.log("Data received");
        try {
			console.log("GOT DATA");
            outStream.write(data);
            
            

			//outStream.flush();
			console.log("got chunk of " + data.toString('hex'));
		}
        catch (ex) {
            console.error("Er!" + ex);
        }
    });
});

// use : 
// setTimeout(stopStream, 10 * 1000);


function stopStream(){
    console.log("closingin in 10 seconds");
    outStream.end();
}
