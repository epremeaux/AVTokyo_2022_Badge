'use strict';
const fs = require('fs');
const hostname = 'mqtt://standard.mqttserver'; 
const mqtt = require('mqtt');
const assert = require('assert');
const net = require('os').networkInterfaces();
const mysql = require('mysql2');
const util = require('util');


let dbconfig = {
  host    : "localhost",
  user    : "dbuser",
  password: "password",
  database: "avtokyodb",
};

var VM = [];

var log_file = fs.createWriteStream('/var/log/avtokyo-server.log', {flags : 'a'});

console.log = function(d) { //
var dt = new Date(); 
var tstamp = '['+IntTwoChars(dt.getHours())+':'+IntTwoChars(dt.getMinutes())+':'+IntTwoChars(dt.getSeconds())+']';
  log_file.write(util.format(tstamp+' '+d) + '\n');
};

var IntTwoChars = function(i) {
return (`0${i}`).slice(-2);
};


function zeroPad(num, places) {
  var zero = places - num.toString().length + 1;
  return Array(+(zero > 0 && zero)).join("0") + num;
};


var formatDate = function(dt) {
	var d = new Date(dt),
			month = '' + (d.getMonth() + 1),
			day = '' + d.getDate(),
			year = d.getFullYear();

	if (month.length < 2) month = '0' + month;
	if (day.length < 2) day = '0' + day;
	return [year, month, day].join('-');
};

function leadZero(n) { return n < 10 ? '0' + n: n;};

var getFullDatetime = function f() {

	var d = new Date();
	var month = d.getMonth()+1;
	var day = d.getDate();
	var hour = d.getHours();
	var minute = d.getMinutes();
	var second = d.getSeconds();
	month = leadZero(month);
	day = leadZero(day);
	hour = leadZero(hour);
	minute = leadZero(minute);
	second = leadZero(second);

	return d.getFullYear()+'-'+month+'-'+day+' '+hour+':'+minute+':'+second;
	
};


var logMessage = function(messageid,message) {

let conn = mysql.createConnection(dbconfig);

let sql = `INSERT INTO tblAVLog(dDateTime,nMessageID,cMessage) VALUES(NOW(),?,?)`;

if(message.length > 254)
	message = message.substring(0,254);

let flds = [messageid,message];

conn.query(sql,flds, (err, results, fields) => {
  if (err) {
    return console.log(err.message);
  }
});

conn.end();

};
 
process.on('SIGTERM', () => {
  console.info('SIGTERM signal received.');
  
  _db.close();
  
  if(conn) conn.close();
  
  if(server) server.end();
  
	console.log('AVTokyo MQTT Server Closed...');

});

var insertSensorData = function(uid,sensor,value) {

let sql = "INSERT tblSensors (cUID,iType,fValue) VALUES(?,?,?);";

let flds = [uid,sensor,value];

conn.query(sql,flds, (err, results, fields) => {
  if (err) {
    return console.log(err.message);
  }
});

}


var insertGameData = function(uid,game,value) {
console.log('Insert '+uid+' '+game+' '+value);
let sql = "INSERT tblGameScore (cUID,nGame,nScore) VALUES(?,?,?);";

let flds = [uid,game,value];

conn.query(sql,flds, (err, results, fields) => {
	console.log(err);
	console.log(results);
  if (err) {
    return console.log(err.message);
  }
});


}

// load authenticated devices then subscribe to their messages

const server  = mqtt.connect(hostname,options);

server.on("error",function(error){
	console.log("Can't connect" + error);
	process.exit(1)
});

server.on('connect', function () {

	console.log('Subscribing to AVTOKYO MQTT');

  server.subscribe('/AVTokyo2022/#', function (err) {
    if (!err) {
        console.log('Subscribed to /AVTokyo2022/');
    }
  });

});

server.on('error', function(err) {
        logMessage(3,err);
    	console.log(err);
});


server.on('message', function (topic, message) {

	//console.log(topic);
	
	if(topic.indexOf('/AVTokyo2022/') == 0 && topic.indexOf('/Sensors/') > 0) { // got a sensor reading
		
		var tops = topic.split('/');
		var uid = tops[2]; // [23423]

		// /AVTokyo2022/[29B898]/Sensors/Humidity
		if(uid.length == 8 && uid[0] == '[') {
			uid = uid.slice(1,-1);
			var sensor = 0;
			// INSERT into tblSensors (cUID,iType,fValue) (uid,sensor,message)
			if(tops[4].indexOf('Voltage') >= 0)
				sensor = 1;
			if(tops[4].indexOf('Temperature') >= 0)
				sensor = 2;
			if(tops[4].indexOf('Humidity') >= 0)
				sensor = 3;
			//console.log(uid+' '+sensor+' '+message);
			insertSensorData(uid,sensor,parseFloat(message));
			
		}		
		return;
	}

	if(topic.indexOf('/AVTokyo2022/') == 0 && topic.indexOf('/Games/') > 0) { // got a sensor reading
		
		var tops = topic.split('/');
		var uid = tops[2]; // [23423]

		// /AVTokyo2022/[29B898]/Games/Quiz
		if(uid.length == 8 && uid[0] == '[') {
			uid = uid.slice(1,-1);
			var game = 0;
			// INSERT into tblSensors (cUID,iType,fValue) (uid,sensor,message)
			if(tops[4].indexOf('Snek') >= 0)
				game = 1;
			if(tops[4].indexOf('Quiz') >= 0)
				game = 2;
			console.log(uid+' '+game+' '+message);
			insertGameData(uid,game,parseInt(message));

			
		}		
	
	}


});

