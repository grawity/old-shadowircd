require 'socket'

$SID = "5AB"

sock = TCPSocket.new("irc.nenolod.net", 8887)
sock.puts "PASS password TS #{Time.now.to_s} :#{$SID}"
sock.puts "SID kageserv.shadowircd.net 1 #{$SID} :KageServ Services"

sock.each_line do
  |line|
  puts line.gsub(/[\r\n]+/,'')
end
