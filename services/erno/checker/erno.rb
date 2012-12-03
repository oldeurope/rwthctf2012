#!/usr/bin/ruby

class Perm
	def initialize(p)
		@p = p
		@n = p.length
	end

	def [](i)
		@p[i-1]
	end

	def []=(i,j)
		@p[i-1] = j
	end

	def cycle(c)
		s = self
		c.each_index do |i|
			t = Perm.new([*1..@n])
			c[i].each_index do |j|
				t[c[i][j]] = c[i][(j+1) % c[i].length]
			end
			s = s * t
		end
		s
	end

	def *(x)
		Perm.new([*1..@n].map{ |i| self[x[i]] })
	end

	def to_s
		@p.inspect
	end
end


class Cube
	def initialize(moves=[])
		@state = Perm.new([*1..48])
		@gen = [
			[ [ 1, 3, 8, 6], [ 2, 5, 7, 4], [ 9,33,25,17], [10,34,26,18], [11,35,27,19] ], # U
			[ [ 9,11,16,14], [10,13,15,12], [ 1,17,41,40], [ 4,20,44,37], [ 6,22,46,35] ], # L
			[ [17,19,24,22], [18,21,23,20], [ 6,25,43,16], [ 7,28,42,13], [ 8,30,41,11] ], # F
			[ [25,27,32,30], [26,29,31,28], [ 3,38,43,19], [ 5,36,45,21], [ 8,33,48,24] ], # R
			[ [33,35,40,38], [34,37,39,36], [ 3, 9,46,32], [ 2,12,47,29], [ 1,14,48,27] ], # B
			[ [41,43,48,46], [42,45,47,44], [14,22,30,38], [15,23,31,39], [16,24,32,40] ]  # D
		].map{ |c| @state.cycle(c) }
		apply(moves)
	end

	def apply(moves=[])
		moves.each do |move|
			@state = @state * @gen[move]
		end
	end

	def to_i
		[*1..48].inject(0){ |r,i| r + (@state[i]-1) * (48**(i-1)) }
	end

	def to_s
		@state.to_s
	end
end


class Cayley
	def self.hash(m)
		m = m.unpack("C*")
		c = Cube.new
		if m.length >= 1
			m = [*1..m.length].inject(0){ |r,i| r + (m[i-1] * 256**(i-1)) }
			while m != 0
				c.apply([m%6])
				m /= 6
			end
		end
		c.to_i
	end
end
