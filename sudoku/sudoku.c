#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "sudoku_def.h"


SudokuTable_t SudokuTable;

#ifdef debug
void can_num_table_debug();
#endif

void double_link_add_after(DoubleLink_t *pTo, DoubleLink_t *pAdd){
	pAdd->pNext = pTo->pNext;
	pAdd->pNext->pPrev = pAdd;
	pAdd->pPrev = pTo;
	pTo->pNext = pAdd;
}

void double_link_delete(DoubleLink_t *pDelete){
	pDelete->pNext->pPrev = pDelete->pPrev;
	pDelete->pPrev->pNext = pDelete->pNext;
}

void board_horizontal_line_print(){
	// ループ用変数
	int i,j;

	// 区切りを表示
	for(i = 0;i < BOARD_M;i++){
		printf("+-");
		for(j = 0;j < BOARD_N;j++){
			printf("--");
		}
	}
	printf("+\n");
}

bool is_print_horizontal_position(int pos,int box_size){
	// 区切りを表示すべき場所の場合は1を、表示すべきでない場所の場合は0をreturn
	if( (pos + 1) % box_size == 0 )
		return true;
	else
		return false;
}

void board_print(){
	// ループ用変数
	int i,j;

	// 横線を表示
	board_horizontal_line_print();
	// 縦の列でループを回す
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		// 最初の区切り線印字
		printf("|");
		// 横の列でループを回す
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			//空白をprint
			printf(" ");
			// 値を表示(ただし、値が0の場合は、「.」を表示)
			if(SudokuTable.MainBoard[i][j].Value != 0)
				printf("%d",SudokuTable.MainBoard[i][j].Value);
			else
				printf(".");
			// 区切りを表示すべき場所の場合表示
			if(is_print_horizontal_position(j,BOARD_N)) printf(" |");
		}
		printf("\n");
		// 区切りの横棒を表示すべき場所の場合表示
		if(is_print_horizontal_position(i,BOARD_M))
			board_horizontal_line_print();
	}
}

void main_tain_init(){
	// ループ用変数
	int i;
	
	// リンクの初期化
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		SudokuTable.CanNumTable[i].Link.pNext = &SudokuTable.CanNumTable[i].Link;
		SudokuTable.CanNumTable[i].Link.pPrev = &SudokuTable.CanNumTable[i].Link;
	}
}

void board_group_init(){
	// ループ用変数
	int i,j,k,l;
	// Index保持用変数
	int grpidx,cellidx;

	// Index保持用変数初期化
	grpidx = cellidx = 0;

	// 横向きのグループ作成
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			// Cellをグループへ紐づけ
			SudokuTable.Groups[grpidx].BoardTable[j] = &SudokuTable.MainBoard[i][j];
			// グループをCellに紐づけ
			SudokuTable.MainBoard[i][j].AssociatedGroups[0] = &SudokuTable.Groups[grpidx];
		}
		// グループのインデックス更新
		grpidx++;
	}
	
	// 縦向きのグループ作成
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			// Cellをグループへ紐づけ
			SudokuTable.Groups[grpidx].BoardTable[j] = &SudokuTable.MainBoard[j][i];
			// グループをCellに紐づけ
			SudokuTable.MainBoard[j][i].AssociatedGroups[1] = &SudokuTable.Groups[grpidx];
		}
		// グループのインデックス更新
		grpidx++;
	}
	
	// 3x3マスのグループ作成
	
	// 3x3マスの縦方向のループ
	for(l = 0;l < BOARD_N * BOARD_M;l += BOARD_N){
		// 3x3マスの横方向のループ
		for(k = 0;k < BOARD_N * BOARD_M;k += BOARD_M){
			// グループの中の、マスのインデックス
			cellidx = 0;
			// 3x3マスの縦向きのループ
			for(i = 0;i < BOARD_M;i++){
				// 3x3マスの横向きのループ
				for(j = 0;j < BOARD_N;j++){
					// Cellをグループへ紐づけ
					SudokuTable.Groups[grpidx].BoardTable[cellidx] = &SudokuTable.MainBoard[l+i][k+j];
					// グループをCellに紐づけ
					SudokuTable.MainBoard[l+i][k+j].AssociatedGroups[2] = &SudokuTable.Groups[grpidx];
					// グループ内のマスのインデックスの更新
					cellidx++;
				}
			}
			// グループのインデックスの更新
			grpidx++;
		}
	}
}

void candidate_init(){
	// ループ用の変数
	int i,j,k;
	// セル単位でループ
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			// Candidate配列単位でループ
			for(k = 0;k < BOARD_N * BOARD_M;k++){
				SudokuTable.MainBoard[i][j].Candidate[k] = SudokuTable.MainBoard[i][j].Value == 0;
			}
#ifdef debug
			SudokuTable.MainBoard[i][j].x = i+1;
			SudokuTable.MainBoard[i][j].y = j+1;
#endif
		}
	}
}

void board_read(const char *filename){
	// File Pointerを作成
	FILE *fp;

	// 読みだしたデータの保管用
	char buf[BOARD_N * BOARD_M + 1];

	// ループ用の変数
	int i,j;

	// sprintfフォーマット保管用
	char sprintf_format[16];

	// 読み出した値を一時保管する変数
	int cache_value;

	// ファイルオープン
	if( (fp = fopen(filename,"r")) == NULL ){
		printf("ファイルのオープンに失敗しました。\n");
		exit(EXIT_FAILURE);
	}

	// spirntfのフォーマットとなる文字列を生成。
	sprintf(sprintf_format,"%%%ds%%*[^\n]",BOARD_N * BOARD_M);

	// %で始まる最初の行を読み飛ばす。
	fscanf(fp,"%*[^\n]");

	// 1行ずつ読み込む
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		// データの読み込み/エラーチェックを行う
		if(fscanf(fp,sprintf_format,buf) == EOF){
			// EOFの場合、必要行数がないことを報告し、終了
			printf("sudoku_def.hで、指定された量だけのデータが、ボードファイルに存在しません。\n");
			exit(EXIT_FAILURE);
		}
		// 一文字ずつ読み込む
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			if(buf[j] == '.'){
				// .の場合、0を代入
				cache_value = 0;
			}else if('0' <= buf[j] && buf[j] <= '9'){
				// 0-9の場合、0を引いた値を代入
				cache_value = buf[j] - '0';
			}else if('a' <= buf[j] && buf[j] <= 'z'){
				cache_value = buf[j] - 'a' + 10;
			}else if('A' <= buf[j] && buf[j] <= 'Z'){
				cache_value = buf[j] - 'A' + 10;
			}else if(buf[j] == '\0'){
				// NULL文字の場合、文字が足りないことを報告し、終了
				printf("sudoku_def.hで、指定された量だけのデータが、ボードファイルに存在しません。\n");
				exit(EXIT_FAILURE);
			}else{
				// それ以外の場合、不正な値が入力されたことを報告し、終了
				printf("不正な値が入力されました。%d,%d,%d\n",buf[j],i,j);
				exit(EXIT_FAILURE);
			}
			//正常な値かを確認
			if(0 <= cache_value && cache_value <= BOARD_N * BOARD_M){
				// 正常な値だった場合は代入
				SudokuTable.MainBoard[i][j].Value = cache_value;
				// 初期値であるフラグを立てる(デバッグ用)
				SudokuTable.MainBoard[i][j].InitValue = (cache_value != 0);
			}else{
				// それ以外の場合、不正な値が入力されたことを報告し、終了
				printf("不正な範囲の値が入力されました。%d,%d,%d\n",buf[j],i,j);
				exit(EXIT_FAILURE);
			}
		}
	}

	// ファイルをクローズ
	fclose(fp);
}

void board_size_valid(){
	// 2未満の場合、数独が成り立たないため、弾く。
	if(BOARD_N < 2 || BOARD_M < 2){
		printf("ヘッダファイルのボード指定サイズが2未満です。\n");
		exit(EXIT_FAILURE);
	}
}

void candidate_set_by_cell(Cell_t **set_cell,Cell_t **min_cand_cell,int *min_cand_cell_count){
	// ループ用変数
	int i,j,k;
	// カウンター
	int candidate_counter;
	
	// カウンタを最大値で初期化
	candidate_counter = BOARD_N * BOARD_M;
	
	// 探す値の決定
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		(*(*set_cell)).Candidate[i] = (*(*set_cell)).Value == 0;
		// AssociatedGroups単位でのループ
		for(j = 0;j < 3;j++){
			// グループ内でのCellループ
			for(k = 0;k < BOARD_N * BOARD_M;k++){
				// 探している値と等しいかを確認
				if((*(*(*(*set_cell)).AssociatedGroups[j]).BoardTable[k]).Value == i+1){
					// カウンタの値を下げる
					candidate_counter --;
					// flagも下げる
					(*(*set_cell)).Candidate[i] = false;
					// ループを2段抜ける
					break;
				}
			}
			// 2つめのループも抜ける
			if((*(*set_cell)).Candidate[i] == false) break;
		}
	}
	// 今までの値よりもCandidateCount小さい場合は、それを代入
	if(candidate_counter < *min_cand_cell_count){
		*min_cand_cell = &(*(*set_cell));
		*min_cand_cell_count = candidate_counter;
	}
}

void candidate_set(){
	// ループ用変数
	int i,j,k,l,m;
	// カウント用内部変数
	int candidate_counter;

	// Cell単位でループ
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			// マスに何も入っていない場合は論外なので条件分岐で外す。
			if(SudokuTable.MainBoard[i][j].Value == 0){
				// CandidateCounterの初期化
				candidate_counter = BOARD_N * BOARD_M ;
				// 探すものの決定(探すときは+1必須)
				for(k = 0;k < BOARD_N * BOARD_M;k++){
					// AssociatedGroups単位でのループ
					for(l = 0;l < 3;l++){
						// グループ内の、Cellループ
						for(m = 0;m < BOARD_N * BOARD_M;m++){
							// 探している値と等しいかを確認。
							if((*(*SudokuTable.MainBoard[i][j].AssociatedGroups[l]).BoardTable[m]).Value == k+1){
								// 等しい場合は、candidate_counterを減らし、2段階ループを抜ける。
								candidate_counter --;
								SudokuTable.MainBoard[i][j].Candidate[k] = false;
								break;
							}
						}
						// ループを抜けるための条件分岐
						if(SudokuTable.MainBoard[i][j].Candidate[k] == false) break;
					}
				}
				// Candidateの数を、CanidateCountに記録
				SudokuTable.MainBoard[i][j].CandidateCount = candidate_counter;
				// CellNumMaintain_tに追加
				double_link_add_after(&SudokuTable.CanNumTable[candidate_counter-1].Link,&SudokuTable.MainBoard[i][j].Link);
			}
		}
	}
}

bool is_board_complete(){
	int i,j;
	for(i = 0;i < BOARD_N * BOARD_M;i++)
		for(j = 0;j < BOARD_N * BOARD_M;j++)
			if(SudokuTable.MainBoard[i][j].Value == 0)
				return false;
	return true;
}

bool is_can_board_set_value(){
	int i;
	for(i = 0;i < BOARD_N * BOARD_M;i++)
		if((SudokuTable.CanNumTable[i]).Link.pPrev != (&SudokuTable.CanNumTable[i].Link))
			return true;
	return false;
}

void return_best_cell(Cell_t **best_cell){
	// ループ用変数
	int i;
	// CanNumTable単位でループ
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		// 該当するCanNumTableにセルがない場合次のものへ
		if((SudokuTable.CanNumTable[i]).Link.pPrev == (&SudokuTable.CanNumTable[i].Link)) continue;
		// 値があった場合、それをセットしてreturn
		*best_cell = (Cell_t *)SudokuTable.CanNumTable[i].Link.pNext;
		return;
	}
	// 値がない場合NULLをセットしてreturn
	*best_cell = NULL;
	return;
}

bool solve(){
	//最善のセルを保管しておく変数
	Cell_t *best_cell;
	//Candidateのflagを変更したかを保管する変数
	bool is_candidate_val_change[BOARD_N * BOARD_M * 3];
	//CandidateCountの値が例外的であるか
	bool is_candidate_count_exception = false;
	//ループ用変数
	int i,j,k;

	//ボードが完成している場合、成功を通知
	if(is_board_complete() == true)			return true;
	//これ以上値をセットできない場合は、失敗を通知
	if(is_can_board_set_value() == false)	return false;
	
	//最善のセルを教えてもらう
	return_best_cell(&best_cell);

#ifdef debug
	//最善のセルの情報を表示
	printf("SET CC:%d [%d][%d]\n",(*best_cell).CandidateCount,(*best_cell).y,(*best_cell).x);
#endif

	//ループ(値決め)
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		//その値が入れられる場合
		if((*best_cell).Candidate[i]){
			//ステータスをクリア
			is_candidate_count_exception = false;
			for(j = 0;j < BOARD_N * BOARD_M * 3;j++) is_candidate_val_change[j] = false;
			//値をセットしてみる
			(*best_cell).Value = i + 1;
			//候補から抜いてみる
			double_link_delete(&(*best_cell).Link);

			//そのマスのAssociatedGroupsの変更のためにGroup単位でループ
			for(j = 0;j < 3;j++){
				//グループ内のセルの探索
				for(k = 0;k < BOARD_N * BOARD_M;k++){
					//そのセルに値が入っていない場合
					if((*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Value == 0){
						//そのセルの今回置いた値のCandidateがTRUEの場合
						if((*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Candidate[i]){
							//そのセルが今回置いた値のCandidateがTRUEなのに、それが唯一のものでない場合
							if((*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount != 1){
								//Candidateを下げる
								(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Candidate[i] = false;
								//CandidateCountも下げる
								(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount--;
								//DoubleLinkを削除
								double_link_delete(&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
								//DoubleLinkをその場所に指定
								double_link_add_after(&SudokuTable.CanNumTable[(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount].Link,&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
								//変更したというflagを上げる
								is_candidate_val_change[j*BOARD_N*BOARD_M+k] = true;
							}else{
								//異常であるflagを上げる
								is_candidate_count_exception = true;
							}
						}
					}
					if(is_candidate_count_exception) break;
				}
				if(is_candidate_count_exception) break;
			}
			//異常検知した場合、その答えは間違いなので、continueする
			if(is_candidate_count_exception){
				//AssociatedGroupの変更をリセットするためにGroup単位でループ
				for(j = 0;j < 3;j++){
					//グループ内のセルの探索
					for(k = 0;k < BOARD_N * BOARD_M;k++){
						//そのセルの情報に変更が加えられている場合
						if(is_candidate_val_change[j*BOARD_N*BOARD_M+k]){
							//Candidateを下げる
							(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Candidate[i] = true;
							//CandidateCountも下げる
							(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount++;
							//DoubleLinkを削除
							double_link_delete(&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
							//DoubleLinkをその場所に指定
							double_link_add_after(&SudokuTable.CanNumTable[(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount].Link,&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
							//変更したというflagを上げる
							is_candidate_val_change[j*BOARD_N*BOARD_M+k] = true;
						}
					}
				}
#ifdef debug
					printf("REM [%d][%d]\n",(*best_cell).y,(*best_cell).x);
#endif
				//仮に入れてみたやつを消す
				(*best_cell).Value = 0;
				//候補から抜いちゃったやつを戻す
				double_link_add_after(&SudokuTable.CanNumTable[(*best_cell).CandidateCount].Link,&(*best_cell).Link);
			}else{
				if(solve()){
					return true;
				}else{

					//AssociatedGroupの変更をリセットするためにGroup単位でループ
					for(j = 0;j < 3;j++){
						//グループ内のセルの探索
						for(k = 0;k < BOARD_N * BOARD_M;k++){
							//そのセルの情報に変更が加えられている場合
							if(is_candidate_val_change[j*BOARD_N*BOARD_M+k]){
								//Candidateを下げる
								(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Candidate[i] = true;
								//CandidateCountも下げる
								(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount++;
								//DoubleLinkを削除
								double_link_delete(&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
								//DoubleLinkをその場所に指定
								double_link_add_after(&SudokuTable.CanNumTable[(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).CandidateCount].Link,&(*(*(*best_cell).AssociatedGroups[j]).BoardTable[k]).Link);
								//変更したというflagを上げる
								is_candidate_val_change[j*BOARD_N*BOARD_M+k] = true;
							}
						}
					}
#ifdef debug
					printf("REM [%d][%d]\n",(*best_cell).y,(*best_cell).x);
#endif
					//仮に入れてみたやつを消す
					(*best_cell).Value = 0;
					//候補から抜いちゃったやつを戻す
					double_link_add_after(&SudokuTable.CanNumTable[(*best_cell).CandidateCount].Link,&(*best_cell).Link);
					continue;
				}
			}
		}
	}

	//失敗を通知
	return false;
}

#ifdef debug
void can_num_table_debug(){
	int i,j;
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		if(SudokuTable.CanNumTable[i].Link.pNext != &SudokuTable.CanNumTable[i].Link){
			printf("[%d]\n",i+1);
			Cell_t *cache_cell;
			cache_cell = (Cell_t *)(SudokuTable.CanNumTable[i].Link.pNext);
			while(1){
				printf("  [%d][%d] CC:%d ",(*cache_cell).y,(*cache_cell).x,(*cache_cell).CandidateCount);
				for(j = 0;j < BOARD_N * BOARD_M;j++){
					if((*cache_cell).Candidate[j]) printf("%d ",j+1);
				}
				printf("\n");
				if((*cache_cell).Link.pNext == &SudokuTable.CanNumTable[i].Link) break;
				cache_cell = (Cell_t *)((*cache_cell).Link.pNext);
			}
		}
	}
}
#endif
void board_print_for_ruby_program(){
	// ループ用変数
	int i,j;
	// 縦の列でループを回す
	for(i = 0;i < BOARD_N * BOARD_M;i++){
		// 横の列でループを回す
		for(j = 0;j < BOARD_N * BOARD_M;j++){
			printf("%d",SudokuTable.MainBoard[i][j].Value);
			if(SudokuTable.MainBoard[i][j].InitValue)
				printf("t");
			else
				printf("f");
			if(BOARD_N * BOARD_M - 1 != j) printf(":");
		}
		if(BOARD_N * BOARD_M - 1 != i) printf(",");
	}
}


int main(int argc,char *argv[]){
	// ボードのサイズを確認
	board_size_valid();

	// 引数の量を確認
	if(argc == 2){
		// ボードの読み込み
		board_read(argv[1]);

		// 問題の表示
		//board_print();

		// ボードのAssociatedGroupsやGroupsの初期化
		board_group_init();
		// CanNumTableのLinkの初期化
		main_tain_init();
		// Candidateの初期化
		candidate_init();
		// Candidateへのデータセット(main_tain_initに依存)
		candidate_set();
		
#ifdef debug
		can_num_table_debug();
#endif
		
		// 再起呼び出し
		solve();
		
		//board_print();
		board_print_for_ruby_program();
	}else if(argc == 1){
		printf("コマンドライン引数にボードファイルを指定してください。\n");
		exit(EXIT_FAILURE);
	}else{
		printf("コマンドライン引数が多すぎます。\n");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

