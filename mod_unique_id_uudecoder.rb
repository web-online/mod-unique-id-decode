#!/usr/bin/env ruby

require 'ipaddr'
require 'optparse'

class UniqueID < Struct.new(:stamp, :ip, :pid, :counter, :thread_index)
  def to_s
    sprintf("unique_id.stamp = %s\n", stamp.ctime) +
    sprintf("unique_id.in_addr = %s\n", ip) +
    sprintf("unique_id.pid = %d\n", pid) +
    sprintf("unique_id.counter = %d\n", counter) +
    sprintf("unique_id.thread_index = %d\n", thread_index)
  end
end

$uuencoder = [
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', '-',
]

def unique_id_uudecode(s_unique_id)
  struct_format = 'NNNnN'
  unique_id_rec_total_size = Array.new(5, 0).pack(struct_format).length
  unique_id_rec_size_uu = (unique_id_rec_total_size*8+5)/6
  index = [0,0,0,0]
  k = 0
  y = Array.new(unique_id_rec_total_size, 0)
  (0..unique_id_rec_total_size-1).step(3) do |i|
    index[0] = $uuencoder.index(s_unique_id[k]); k += 1
    index[1] = $uuencoder.index(s_unique_id[k]); k += 1
    # first 6 bits + next 2 bits
    y[i] = ((index[0] << 2) & 0xfc) | ((index[1] >> 4) & 0x03)

    break if k == unique_id_rec_size_uu
    index[2] = $uuencoder.index(s_unique_id[k]); k += 1
    # remaining 4 bits + next 4 bits
    y[i+1] = ((index[1] << 4) & 0xf0) | ((index[2] >> 2) & 0x0f);

    break if k == unique_id_rec_size_uu
    index[3] = $uuencoder.index(s_unique_id[k]); k += 1
    # remaining 2 bits + next 6 bits (& 0x3f probably not necessary)
    y[i+2] = ((index[2] << 6) & 0xc0) | (index[3] & 0x3f);
  end
  s = y.pack('C*').unpack(struct_format)
  UniqueID.new(Time.at(s[0]), IPAddr.new(s[1], Socket::AF_INET), s[2], s[3], s[4])
end

def main
  options = {}
  OptionParser.new do |opts|
    opts.banner = "Usage: mod_unique_id_uudecoder.rb <options>"

    opts.on(:REQUIRED, "-i","--id unique_id", "unique_id to parse") do |id|
      options[:id] = id
    end
  end.parse!
  unless options[:id].nil?
    puts unique_id_uudecode(options[:id])
  end
end

if __FILE__ == $0
  main
end
