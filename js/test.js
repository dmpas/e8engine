E8 = require("./e8core.js");
E8 = E8.E8;

var s123 = E8.Create.String("123");
var n123 = E8.Create.Number(123);
var d123 = E8.Create.Date(Date.parse("2015-08-06 15:14:00"));

console.log(s123.eq(n123));
console.log(s123.ne(n123));
console.log(s123.eq(s123));
console.log(n123.ne(n123));
console.log(n123.eq(n123));

var v = new E8.Variant();
v["Ф"] = function () { return 2; }
console.log(v["Ф"]());
