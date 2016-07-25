require 'bundler/setup'
Bundler.require
require 'sinatra/reloader' if development?
require 'google_url_shortener'

def shorten_url long_url
	Google::UrlShortener::Base.api_key = "AIzaSyCyG8yi5WQSrdk6wjw9i_NXSkDXDXIeI0g";
	Google::UrlShortener::shorten!(long_url)
end

def sudoku_compile
	unless File.exists?("./sudoku/sudoku.exe")
		`make -C sudoku`
	end
end

get "/*" do
	sudoku_compile
	pass
end
post "/*" do
	sudoku_compile
	pass
end

get "/" do
	@autoreload = params[:ar] == "yes"
	erb :index
end

get "/board-set-page" do
	erb :board_set_page
end

get "/process" do
	#GETの/processにアクセスが来た場合エラー
	"Error : /processは、POSTでのリクエストのみを受け付けます。"
end

get "/share" do
	unless /\A\d+_\w{32}\.board\Z/.match(params[:bfile])
		halt "Error : ファイル名が異常です。"
	end
	unless File.exist?("./boards/#{params[:bfile]}")
		halt "Error : そのファイルは存在しません。"
	end
	@filename = params[:bfile]
	sudoku_engine_return_str = `./sudoku/sudoku.exe boards/#{@filename}`
	sudoku_engine_return_vals = sudoku_engine_return_str.split("\n").delete_at(-1)

	if sudoku_engine_return_vals.kind_of?(Array)
		@answers = Array.new(sudoku_engine_return_vals.length){Array.new(9){Array.new(9){Array.new(2)}}}
	else
		@answers = Array.new(1){Array.new(9){Array.new(9){Array.new(2)}}}
	end
	
	unless sudoku_engine_return_vals.kind_of?(Array)
		cache_val = sudoku_engine_return_vals
		sudoku_engine_return_vals = Array.new(1)
		sudoku_engine_return_vals[0] = cache_val
	end
	
	sudoku_engine_return_vals.each_with_index do |return_ans,i|
		return_ans_row_datas = return_ans.split(",")
		return_ans_row_datas.each_with_index do |row_data,j|
			cell_datas = row_data.split(":")
			cell_datas.each_with_index do |cell_data,k|
				@answers[i][j][k][0] = cell_data[0]
				@answers[i][j][k][1] = cell_data[1] == "t"
			end
		end
	end
	@ref = "share"
	erb :answer_view
end
post "/process" do
	t0 = (Time.now.to_i * 1000 * 1000) + Time.now.usec;

	#POSTで飛んできたデータを変数に代入
	sudoku_data_param = params[:sudoku_data]

	#nil判定。
	if sudoku_data_param.nil?
		#nilだった場合はエラー
		halt "Error : パラメーター sudoku_data がPOSTリクエストに存在しません。"
	end

	#文字数バリデーション
	unless sudoku_data_param.length == ( 9 + 1 ) * 9 - 1
		#一致しなかった場合はエラー
		halt "Error : パラメーター sudoku_data の全体の文字数が異常です。"
	end

	#正規表現で文字バリデーション
	unless /\A(\.1-9,)+\z/.match(sudoku_data_param).nil?
		#nilだった場合はエラー
		halt "Error : パラメーター sudoku_data に/\A(\.1-9,)+\z/にマッチしないものが含まれています。"
	end

	#sudoku_data_paramをArrayに変換
	sudoku_data_ary = sudoku_data_param.split(",")

	#Lengthでバリデーション
	unless sudoku_data_ary.length == 9
		#9ではなかった場合はエラー
		halt "Error : パラメーター sudoku_data の行数が異常です。"
	end

	#Arrayの文字列の長さでバリデーション
	sudoku_data_ary.each do |sdata|
		unless sdata.length == 9
			#こちらも0ではなかった場合はエラー
			halt "Error : パラメーター sudoku_data の列数が異常です。"
		end
	end

	#ボードフォルダの存在を確認
	unless Dir.exist?("./boards")
		#存在しない場合作成
		`mkdir boards`
	end

	#作成するボードファイルの名前を作成
	str_ary = ('a'..'z').to_a + ('A'..'Z').to_a + ('0'..'9').to_a;
	rand_str = (Array.new(32){str_ary[rand(str_ary.size)]}).join
	@filename = Time.now.to_i.to_s + "_" + rand_str + ".board"

	#ボードファイル作成
	File.open("./boards/" + @filename,"w") do |file|
		#1行目にコメントを入力(リクエスト元IP)
		file.puts("%" + " TIME:" + Time.now.to_s + " IP:" + request.ip)
		#ファイルに1行ずつ書き込む
		sudoku_data_ary.each do |sdata|
			file.puts(sdata)
		end
	end

	t1 = (Time.now.to_i * 1000 * 1000) + Time.now.usec;
	
	sudoku_engine_return_str = `./sudoku/sudoku.exe boards/#{@filename}`
	sudoku_engine_return_vals = sudoku_engine_return_str.split("\n").delete_at(-1)
	if sudoku_engine_return_str == ""
		halt "問題の入力に間違いはありませんか？もう一度ご確認ください。"
	end
	if sudoku_engine_return_vals.kind_of?(Array)
		@answers = Array.new(sudoku_engine_return_vals.length){Array.new(9){Array.new(9){Array.new(2)}}}
	else
		@answers = Array.new(1){Array.new(9){Array.new(9){Array.new(2)}}}
	end
	
	unless sudoku_engine_return_vals.kind_of?(Array)
		cache_val = sudoku_engine_return_vals
		sudoku_engine_return_vals = Array.new(1)
		sudoku_engine_return_vals[0] = cache_val
	end
	sudoku_engine_return_vals.each_with_index do |return_ans,i|
		return_ans_row_datas = return_ans.split(",")
		return_ans_row_datas.each_with_index do |row_data,j|
			cell_datas = row_data.split(":")
			cell_datas.each_with_index do |cell_data,k|
				@answers[i][j][k][0] = cell_data[0]
				@answers[i][j][k][1] = cell_data[1] == "t"
			end
		end
	end

	erb :answer_view
end
get "/short_url" do
	if params[:filename].nil?
		halt "Error : ファイルの指定パラメーターがありません。"
	end
	if params[:filename] == ""
		halt "Error : ファイルの指定パラメーターが空です。"
	end
	if params[:hostname].nil?
		halt "Error : ホスト名指定がありません。"
	end
	if params[:hostname] == ""
		halt "Error : ホスト名指定パラメーターが空です。"
	end
	file = params[:filename]
	host = params[:hostname]
	shorten_url "https://#{host}/share?bfile=#{file}"
end

