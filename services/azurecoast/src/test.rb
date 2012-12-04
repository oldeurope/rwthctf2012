require './cellnum.rb'

def crypt(val, key)
  (0..16).each do |i|
    i*=2
    key = key + 1337
    val = val + (key.roll(i+0))
    val = val ^ (key.loll(i+1))
  end
  return val
end

def hash(buffer)
  buffer.map{|x| cell(x)}
  int = cell(0x92345678)
  buffer.each do |i|
    int = crypt(int,i*i+i)
  end
  return int
end

puts hash([0]).to_s(16)
puts hash([1]).to_s(16)
puts hash([2]).to_s(16)
puts hash([120,45,45,45,6]).to_s(16)
puts hash([121,45,45,44,6]).to_s(16)
puts hash([121,45,45,45,7]).to_s(16)
puts hash([121,45,45,45,8]).to_s(16)
puts hash([121,45,45,45,9]).to_s(16)
