require 'json'

File.open("setting.json", "r") {|f|
	rdata = f.read()
	p JSON.parse(rdata)
}
