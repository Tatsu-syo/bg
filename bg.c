#include	<tchar.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<windows.h>
#include	<winbase.h>
#include	<direct.h>

//
int	wait = 0;		// �N���R�}���h�̏I���҂��v���Z�X��?
int	notify = 0;		// �ʒm����
DWORD dwCreationFlags = 0;

/**
 * �I�v�V��������͂���
 *
 * @param argc �����̐�
 * @param argv �����z��
 * @return �����̏I���ʒu
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
					// �ʒm����̏ꍇ
					notify = 1;
				break;
				case _T('w'):
					// �����N���Ńv���Z�X�I���҂�
					wait = 1;
				break;
				case _T('d'):
					// �N�������v���Z�X���R���\�[���ɃA�N�Z�X�����Ȃ��B
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

	// �S���I�v�V�����Ŋ̐S�ȃR�}���h���C�����Ȃ�
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
		// �I�v�V����������Ȃ��ꍇ�̓G���[�Ƃ���B
		exit(0);
	}

	if (wait) {	// �������Ȃ����B
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
		_tcscpy(argbuf,_T("bg -w"));	// �E�F�C�g�w��Ŏ������ė����グ����B
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
					NULL,	// �n���h���̃v���Z�X�ւ̌p���Ɋւ���Z�L�����e�B����
					NULL,	// �n���h���̃X���b�h�ւ̌p���Ɋւ���Z�L�����e�B����
					TRUE,	// �n���h�����p������
					dwCreationFlags | NORMAL_PRIORITY_CLASS,	//�D�揇�ʂƃv���C�I���e�B
					NULL,	// �Ăяo�����̊��u���b�N���g���Ċ����p������B
					(LPCTSTR)dir,	// �J�����g�f�B���N�g��
					&start_info,
					&proc_info);

	if (result){	// �v���Z�X�͖����N�������B
		if (wait){	// �v���Z�X�I����҂�
			WaitForSingleObject(proc_info.hProcess,INFINITE);
			_ftprintf(stdout, _T("\n(%u) Done %s\n"), proc_info.dwProcessId, argbuf);
		}

		// �n���h�����c��Ȃ��悤�N���[���A�b�v
		CloseHandle(proc_info.hProcess);
		CloseHandle(proc_info.hThread);

	}
	free(argbuf);
	free(dir);

	return 0;
}
