var E8 = (function () {
	var VARIANT = function() {
		var result = {
			type: "Undefined", value: undefined,
			
			set_as: function (type, value) {
				this.type = type;
				this.value = value;
				return this;
			},
			
			eq: function (v2) {
				
				if (this.type !== v2.type) {
					return false;
				}
				
				if (this.value !== v2.value) {
					return false;
				}
				
				return true;
			},
			ne: function (v2) {
				return !this.eq(v2);
			},
			le: function (v2) {
				
				if (this.type !== v2.type) {
					throw "Incompatible types";
				}
					
				if (this.type === "Number"
					|| this.type === "String"
					|| this.type === "Date"
					|| this.type === "Boolean"
				) {
					return this.value <= v2.value;
				}
					
				return false;
			},
			ge: function (v2) {
				
				if (this.type !== v2.type) {
					throw "Incompatible types";
				}
					
				if (this.type === "Number"
					|| this.type === "String"
					|| this.type == "Date"
					|| this.type == "Boolean"
				) {
					return this.value >= v2.value;
				}
					
				return false;
			},
			lt: function (v2) {
				
				if (this.type !== v2.type) {
					throw "Incompatible types";
				}
					
				if (this.type === "Number"
					|| this.type === "String"
					|| this.type === "Date"
					|| this.type === "Boolean"
				) {
					return this.value < v2.value;
				}
					
				return false;
			},
			gt: function (v2) {
				
				if (this.type !== v2.type) {
					throw "Incompatible types";
				}
					
				if (this.type === "Number"
					|| this.type === "String"
					|| this.type === "Date"
					|| this.type === "Boolean"
				) {
					return this.value > v2.value;
				}
					
				return false;
			},

			convertToString: function() {
				return "" + this.value;
			},
			
			assign: function (v2) {
				this.type = v2.type;
				this.value = v2.value;
				return this;
			},
			
			add_nn: function (n) {
				this.value += n;
				return this;
			},
			
			add_s: function (any) {
				this.value += any; // TODO: Cast
				return this;
			},
			
			add_dn: function (n) {
				this.value += n*1000;
				return this;
			},
			
			add: function (v2) {
				return this;
			}
			
		};
		return result;
	};

	var Create = {
		Number: function (value) {
			var v = new VARIANT();
			return v.set_as("Number", value);
		},
		String: function (value) {
			var v = new VARIANT();
			return v.set_as("String", value);
		},
		Date: function (value) {
			var v = new VARIANT();
			return v.set_as("Date", value);
		},
		Boolean: function (value) {
			var v = new VARIANT;
			return v.set_as("Boolean", value);
		}
	};

	var Functions = {
		Bool: function (value) {
			return value ? Create.Boolean(true) : Create.Boolean(false); 
		},
		Number: function (value) {
			if (value.type === "String") {
				// TODO: Cast
				return Create.Number(0);
			}
			if (value.type === "Boolean") {
				return Create.Number(value.value ? 1 : 0);
			}
			if (value.type === "Number") {
				return value;
			}
			throw "can't convert to number";
		},
		Str: function (value) {
			return value.convertToString();
		},
	};
	
	return {
		Variant: VARIANT,
		Create: Create,
	};
})();

exports.E8 = E8;

