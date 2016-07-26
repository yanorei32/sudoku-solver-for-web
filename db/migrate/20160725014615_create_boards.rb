class CreateBoards < ActiveRecord::Migration
	def change
		create_table :boards do |t|
			t.string	:short_url
			t.text		:data
			t.timestamps null: false
		end
	end
end
