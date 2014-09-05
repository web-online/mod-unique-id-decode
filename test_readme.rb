#!/usr/bin/env ruby

require 'minitest/autorun'

class TestReadme < MiniTest::Unit::TestCase

  def test_readme
    expected_out = <<eout
unique_id.stamp = Tue Sep  2 15:11:38 2014
unique_id.in_addr = 127.0.0.1
unique_id.pid = 666
unique_id.counter = 63539
unique_id.thread_index = 1
eout

    prev = nil
    in_command = false
    code = nil
    command = nil
    File.read('README.md').each_line do |line|
      code = prev.chomp if line =~ /^------*$/
      if match = line.match(/^### (.*)/)
        command, = match.captures
      end
      if line =~ /^```$/
        if in_command
          puts "\n#{code} -- #{command}"
          out = `#{prev}`
          if command == "run"
            assert_equal expected_out, out
          end
        end
        # toggle boolean
        in_command ^= true
      end
      prev = line
    end
  end

end
