#include	<tchar.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<windows.h>
#include	<winbase.h>
#include	<direct.h>

//
int	wait = 0;		// 起動コマンドの終了待ちプロセスか?
int	notify = 0;		// 通知あり
DWORD dwCreationFlags = 0;

/**
 * オプションを解析する
 *
 * @param argc 引数の数
 * @param argv 引数配列
 * @return 引数の終了位置
 */
int parseOption(int argc,TCHAR *argv[])
{
	int i,j,len;

	for (i = 1;i < argc; i++) {
		if (_tcscmp(argv[i], _T("--")) == 0){
			return i;
		}
		
		if (argv[i][0] != '-') {
			return i;
		}

		len = _tcslen(argv[i]);
		if (len == 1) {
			_ftprintf(stderr, _T("Invalid option.\n"));
			return -1;
		}

		for (j = 1; j < len; j++){
			switch (_totlower(argv[i][j])){
				case _T('n'):
					// 通知ありの場合
					notify = 1;
				break;
				case _T('w'):
					// 内部起動でプロセス終了待ち
					wait = 1;
				break;
				case _T('d'):
					// 起動したプロセスをコンソールにアクセスさせない。
					dwCreationFlags |= DETACHED_PROCESS;
				break;
				case _T('c'):
					dwCreationFlags |= CREATE_NEW_CONSOLE;
				break;
				default:
				break;
			}
		}
	}

	// 全部オプションで肝心なコマンドラインがない
	return -1;
}


int _tmain(int argc,TCHAR *argv[])
{
	int len,i,result,com_start;
	TCHAR *argbuf;
	TCHAR *dir;
	PROCESS_INFORMATION proc_info;
	STARTUPINFO start_info;

	if (argc < 2) {
		_ftprintf(stderr,_T("bg commandline\n"));
		exit(1);
	}
	com_start = parseOption(argc,argv);
	// _ftprintf(stderr,_T("%d\n"), com_start);

	if (com_start == -1) {
		// オプションが合わない場合はエラーとする。
		exit(0);
	}

	if (wait) {	// 矛盾をなくす。
		notify = 0;
	}

	len = 0;
	
	TCHAR *commandLine = (TCHAR *)GetCommandLine();
	TCHAR *commandStart;

	// _ftprintf(stderr, _T("%s\n"), commandLine);

	if (com_start > 0) {
		if (_tcscmp(_T("--"), argv[com_start]) == 0) {
			commandStart = _tcsstr(commandLine, _T("--"));
			commandStart += 3;
		} else {
			commandStart = _tcsstr(commandLine, argv[com_start]);
			// _ftprintf(stderr, _T("%s\n"), argv[com_start]);
		}
	} else {
		commandStart = commandLine;
	}
	// _ftprintf(stderr, _T("Commandline: %s\n"), commandStart);
	
	len = _tcslen(commandStart);

	if (notify) {
		len += 7;
		if (dwCreationFlags & DETACHED_PROCESS){
			len++;
		}
	}

	argbuf = malloc((len + 1) * sizeof(TCHAR));
	if (argbuf == NULL) {
		exit(2);
	}

	if (notify){
		_tcscpy(argbuf,_T("bg -w"));	// ウェイト指定で自分を再立ち上げする。
		if (dwCreationFlags & DETACHED_PROCESS){
			_tcscat(argbuf,_T("d"));
			dwCreationFlags = 0;
		}
		_tcscat(argbuf,_T(" "));
		_tcscat(argbuf, commandStart);
	} else {
		_tcscpy(argbuf, commandStart);
	}
	// _ftprintf(stderr, _T("%s\n"), argbuf);

	dir = _tgetcwd(NULL, MAX_PATH);
	if (dir == NULL){
		free(argbuf);
		exit(2);
	}

	memset(&proc_info,0x00,sizeof(PROCESS_INFORMATION));
	memset(&start_info,0x00,sizeof(STARTUPINFO));
	start_info.cb = sizeof(STARTUPINFO);

	result = CreateProcess(	NULL,
					(LPTSTR)argbuf,
					NULL,	// ハンドルのプロセスへの継承に関するセキュリティ属性
					NULL,	// ハンドルのスレッドへの継承に関するセキュリティ属性
					TRUE,	// ハンドルを継承する
					dwCreationFlags | NORMAL_PRIORITY_CLASS,	//優先順位とプライオリティ
					NULL,	// 呼び出し側の環境ブロックを使って環境を継承する。
					(LPCTSTR)dir,	// カレントディレクトリ
					&start_info,
					&proc_info);

	if (result){	// プロセスは無事起動した。
		if (wait){	// プロセス終了を待つ
			WaitForSingleObject(proc_info.hProcess,INFINITE);
			_ftprintf(stdout, _T("\n(%u) Done %s\n"), proc_info.dwProcessId, argbuf);
		}

		// ハンドルが残らないようクリーンアップ
		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

	}
	free(argbuf);
	free(dir);

	return 0;
}
