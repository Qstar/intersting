var express = require("express");
var app = express();
var path = require("path");

app.use("/styles", express.static(__dirname + '/styles'));
app.use("/scripts", express.static(__dirname + '/scripts'));
app.use(express.static(__dirname + '/'));

app.get('/', function(req, res) {
	res.sendFile(path.join(__dirname + '/index.html'));
});

app.listen(3000);
console.log("Running at Port 3000");