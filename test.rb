def smooth(from, to, speed)
  # to is our amplitude
  i = 0
  loop do 
    y = to * Math::sin(
    i+=1
  end
end


a = [25,36]

# our player
b = [2,3]

delta_x = (b[0] - a[0]).abs
delta_y = (b[1] - a[1]).abs

puts "delta x: #{delta_x}"
puts "delta y: #{delta_y}"

current_yaw = 0.0
yaw = Math::atan2(delta_x, delta_y)
puts "yaw: #{yaw}"

delta_yaw = yaw - current_yaw
puts "delta_yaw: #{delta_yaw}"



