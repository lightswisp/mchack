path = "../release/injector"
Dir.chdir("../release")

loop do 
  $pid = Process.spawn(path)
  Thread.new do 
    sleep 10 
    Process.kill("SIGINT", $pid)
  end
  _pid, _exit = Process.wait2($pid)

  status =  _exit.exitstatus
  break if status != 0

  sleep 15 
end