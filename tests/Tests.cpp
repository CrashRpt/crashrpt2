/*************************************************************************************
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

#include "stdafx.h"
#include "Tests.h"
#include "CrashRpt.h"
#include "Utility.h"
#include "strconv.h"

BOOL g_bRunningFromUNICODEFolder = FALSE; // Are we running from a UNICODE-named folder?
BOOL g_bException = FALSE;
int g_nExceptionType = 0;
BOOL g_bExceptionInWorkerThread = FALSE;
BOOL g_bRunAllTest = FALSE;

DWORD WINAPI CrashThread(LPVOID /*lpParam*/)
{
	// Install exception handlers for this thread
	CrThreadAutoInstallHelper cr_thread_install_helper(0);

	if(g_nExceptionType==-1)
	{
		CR_EXCEPTION_INFO ei;
		memset(&ei, 0, sizeof(ei));
		ei.cb = sizeof(ei);
		ei.code = 0x1234;

		crGenerateErrorReport(&ei);

		TerminateProcess(GetCurrentProcess(), 1);
	}
	else
	{
		// Crash
		crEmulateCrash(g_nExceptionType);
	}

	return 0;
}

// Function forward declarations
std::vector<std::string> explode(std::string str, std::string separators = " \t");
void trim2(std::string& str, const char* szTrim=" \t\n");
int fork();

int CALLBACK CrashCallback(CR_CRASH_CALLBACK_INFO * pInfo)
{
	static int counter = 0;
	pInfo->bContinueExecution = FALSE;
	
	if (counter++ > 0)
		return CR_CB_CANCEL;
	else
		return CR_CB_NOTIFY_NEXT_STAGE;
}

int _tmain(int argc, TCHAR** argv)
{
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    //MessageBox(NULL, _T("abc"), _T("abc"), MB_OK);
    if(argc==1)
    {
        _tprintf(_T("\nDo you want to run tests from a folder containing Chinese characters to test UNICODE compatibility (y/n)?\n"));
        _tprintf(_T("Your choice > "));
        TCHAR szAnswer[1024]=_T("");
#if _MSC_VER>=1400
        _getts_s(szAnswer, 1024);
#else
        _getts(szAnswer);
#endif

        if(_tcscmp(szAnswer, _T("y"))==0 ||
            _tcscmp(szAnswer, _T("Y"))==0)
        {
            // Relaunch this process from another working directory containing UNICODE symbols in path.
            // This is needed to test all functionality on UNICODE compatibility.
            g_bRunningFromUNICODEFolder = TRUE;
        }

        if(g_bRunningFromUNICODEFolder)
        {
            _tprintf(_T("Launching tests in another process:\n"));
            if(g_bRunningFromUNICODEFolder)
                _tprintf(_T(" - with working directory having UNICODE symbols in path\n"));
            return fork();
        }
    }
    else
    {
        int argnum;
        for(argnum=1; argnum<argc; argnum++)
        {
            TCHAR* szArg = argv[argnum];
            if(_tcscmp(szArg, _T("/unicode"))==0)
                g_bRunningFromUNICODEFolder = TRUE;
			else if(_tcscmp(szArg, _T("/exception"))==0)
				g_bException = TRUE;
			else if(_tcscmp(szArg, _T("/exception_in_worker_thread"))==0)
				g_bExceptionInWorkerThread = TRUE;
			else if(_tcscmp(szArg, _T("/seh"))==0)
				g_nExceptionType = CR_SEH_EXCEPTION;
			else if(_tcscmp(szArg, _T("/terminate"))==0)
				g_nExceptionType = CR_CPP_TERMINATE_CALL;
			else if(_tcscmp(szArg, _T("/unexpected"))==0)
				g_nExceptionType = CR_CPP_UNEXPECTED_CALL;
			else if(_tcscmp(szArg, _T("/purecall"))==0)
				g_nExceptionType = CR_CPP_PURE_CALL;
			else if(_tcscmp(szArg, _T("/new"))==0)
				g_nExceptionType = CR_CPP_NEW_OPERATOR_ERROR;
			else if(_tcscmp(szArg, _T("/security"))==0)
				g_nExceptionType = CR_CPP_SECURITY_ERROR;
			else if(_tcscmp(szArg, _T("/invparam"))==0)
				g_nExceptionType = CR_CPP_INVALID_PARAMETER;
			else if(_tcscmp(szArg, _T("/sigabrt"))==0)
				g_nExceptionType = CR_CPP_SIGABRT;
			else if(_tcscmp(szArg, _T("/sigfpe"))==0)
				g_nExceptionType = CR_CPP_SIGFPE;
			else if(_tcscmp(szArg, _T("/sigill"))==0)
				g_nExceptionType = CR_CPP_SIGILL;
			else if(_tcscmp(szArg, _T("/sigint"))==0)
				g_nExceptionType = CR_CPP_SIGINT;
			else if(_tcscmp(szArg, _T("/sigsegv"))==0)
				g_nExceptionType = CR_CPP_SIGSEGV;
			else if(_tcscmp(szArg, _T("/sigterm"))==0)
				g_nExceptionType = CR_CPP_SIGTERM;
			else if(_tcscmp(szArg, _T("/manual_report"))==0)
			{
				g_bException = TRUE;
				g_nExceptionType = -1;
			}
            else if (_tcscmp(szArg, _T("/all")) == 0)
            {
                g_bRunAllTest = TRUE;
            }
        }
    }

	if(g_bException && !g_bRunAllTest)
	{
		// Create a temporary folder for wide-char test
		CString sAppDataFolder;
		CString sTmpFolder;
		Utility::GetSpecialFolder(CSIDL_APPDATA, sAppDataFolder);
		sTmpFolder = sAppDataFolder+_T("\\CrashRptExceptionTest");
		Utility::CreateFolder(sTmpFolder);

		// Install crash handler
		CR_INSTALL_INFOW infoW;
		memset(&infoW, 0, sizeof(CR_INSTALL_INFOW));
		infoW.cb = sizeof(CR_INSTALL_INFOW);
		infoW.pszAppVersion = L"1.0.0"; // Specify app version, otherwise it will fail.
		infoW.dwFlags = CR_INST_NO_GUI|CR_INST_DONT_SEND_REPORT|CR_INST_STORE_ZIP_ARCHIVES;
		infoW.pszErrorReportSaveDir = sTmpFolder;

		CrAutoInstallHelper cr_install_helper(&infoW);

		crSetCrashCallback(&CrashCallback, 0);
		if(!g_bExceptionInWorkerThread)
		{
			if(g_nExceptionType==-1)
			{
				CR_EXCEPTION_INFO ei;
				memset(&ei, 0, sizeof(ei));
				ei.cb = sizeof(ei);
				ei.code = 0x1234;

				crGenerateErrorReport(&ei);

				TerminateProcess(GetCurrentProcess(), 1);
			}
			else
			{
				// Crash
				crEmulateCrash(g_nExceptionType);
			}
		}
		else
		{
			// Crash in worker thread
			HANDLE hThread = CreateThread(NULL, 0, CrashThread, 0, 0, NULL);
			WaitForSingleObject(hThread, INFINITE);
		}

		return 1;
	}

    CTestRegistry* pRegistry = CTestRegistry::GetRegistry();
    CTestSuite* pTopSuite = pRegistry->GetTopSuite();
    char szSuiteList[1024] = "";

    if (!g_bRunAllTest)
    {
        printf("\n=== Unit tests for CrashRpt v.%d.%d.%d ===\n\n",
            CRASHRPT_VER / 1000,
            (CRASHRPT_VER % 1000) / 100,
            (CRASHRPT_VER % 1000) % 100);

        // Print the list of test suites

        printf("The list of avaliable test suites:\n");

        UINT nSuiteCount = pTopSuite->GetChildSuiteCount();
        for (UINT i = 0; i < nSuiteCount; i++)
        {
            CTestSuite* pSuite = pTopSuite->GetChildSuite(i);
            std::string sSuiteName;
            std::string sDescription;
            pSuite->GetSuiteInfo(sSuiteName, sDescription);

            printf(" - %s : %s\n", sSuiteName.c_str(), sDescription.c_str());
        }

        printf("\nEnter which test suites to run (separate names by space) or enter empty line to run all test suites.\n");
        printf("Your choice > ");
#if _MSC_VER>=1400
        gets_s(szSuiteList, 1024);
#else
        gets(szSuiteList);
#endif
    }

    // Create the list of test suites to run
    std::string sSuiteList = szSuiteList;
    std::vector<std::string> aTokens = explode(sSuiteList);
    std::set<std::string> aTestSuitesToRun;
    for(UINT i=0; i<aTokens.size(); i++)
        aTestSuitesToRun.insert(aTokens[i]);

    // Determine how many tests to run

    std::vector<std::string> test_list = pTopSuite->GetTestList(aTestSuitesToRun, true);
    size_t nTestsToRun = test_list.size();

    printf("\nRunning tests...\n");

    DWORD dwStartTicks = GetTickCount();

    pTopSuite->Run(aTestSuitesToRun);

    DWORD dwFinishTicks = GetTickCount();
    double dTimeElapsed = (dwFinishTicks-dwStartTicks)/1000.0;

    printf("\n=== Summary ===\n\n");

    // Print all errors (if exist)
    std::vector<std::string> error_list = pTopSuite->GetErrorList(true);
    if(error_list.size()>0)
    {
        printf("Error list:\n");

        for(UINT i=0; i<error_list.size(); i++)
        {
            printf("%d: %s\n", i+1, error_list[i].c_str());
        }
    }

    printf("\n Time elapsed: %0.2f sec.\n", dTimeElapsed);
    printf("   Test count: %d\n", (int)nTestsToRun);
    size_t nErrorCount = error_list.size();
    printf(" Tests passed: %d\n", (int)(nTestsToRun-nErrorCount));
    printf(" Tests failed: %d\n", int(nErrorCount));

    // Wait for key press
    if (!g_bRunAllTest)
        _getch();

    // Return non-zero value if there were errors
    return nErrorCount==0?0:1;
}

// Helper function that removes spaces from the beginning and end of the string
void trim2(std::string& str, const char* szTrim)
{
    std::string::size_type pos = str.find_last_not_of(szTrim);
    if(pos != std::string::npos) {
        str.erase(pos + 1);
        pos = str.find_first_not_of(szTrim);
        if(pos != std::string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

// Helper function that splits a string into list of tokens
std::vector<std::string> explode(std::string str, std::string separators)
{
    std::vector<std::string> aTokens;

    size_t pos = 0;
    for(;;)
    {
        pos = str.find_first_of(separators, 0);

        std::string sToken = str.substr(0, pos);
        if(pos!=std::string::npos)
            str = str.substr(pos+1);

        trim2(sToken);
        if(sToken.length()>0)
            aTokens.push_back(sToken);

        if(pos==std::string::npos)
            break;
    }

    return aTokens;
}


// Launches tests as another process
int fork()
{
    DWORD dwExitCode = 1;
    CString sWorkingFolder;
    CString sCmdLine;
    CString sExeFolder;

    sWorkingFolder = Utility::GetModulePath(NULL);
    sExeFolder = Utility::GetModulePath(NULL);

    if(g_bRunningFromUNICODEFolder)
    {
        CString sAppDataFolder;
		CString sFileName;

        Utility::GetSpecialFolder(CSIDL_LOCAL_APPDATA, sAppDataFolder);
        sWorkingFolder = sAppDataFolder+_T("\\CrashRpt UNICODE 应用程序名称");
        BOOL bCreate = Utility::CreateFolder(sWorkingFolder);
        if(!bCreate)
            return 1;

        /* Copy all required files to temporary directory. */

#ifdef _DEBUG
		sFileName.Format(_T("\\CrashRpt%dd.dll"), CRASHRPT_VER);
        BOOL bCopy = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy)
            goto cleanup;
		sFileName.Format(_T("\\CrashRptProbe%dd.dll"), CRASHRPT_VER);
        BOOL bCopy5 = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy5)
            goto cleanup;
		sFileName.Format(_T("\\CrashSender%dd.exe"), CRASHRPT_VER);
        BOOL bCopy2 = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy2)
            goto cleanup;
        BOOL bCopy4 = CopyFile(sExeFolder+_T("\\Testsd.exe"), sWorkingFolder+_T("\\Testsd.exe"), TRUE);
        if(!bCopy4)
            goto cleanup;
        BOOL bCopy9 = CopyFile(sExeFolder+_T("\\crproberd.exe"), sWorkingFolder+_T("\\crproberd.exe"), TRUE);
        if(!bCopy9)
            goto cleanup;
#else
#ifndef CRASHRPT_LIB
		sFileName.Format(_T("\\CrashRpt%d.dll"), CRASHRPT_VER);
        BOOL bCopy = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy)
            goto cleanup;
		sFileName.Format(_T("\\CrashRpt%d.pdb"), CRASHRPT_VER);
        BOOL bCopy10 = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy10)
            goto cleanup;
		sFileName.Format(_T("\\CrashRptProbe%d.dll"), CRASHRPT_VER);
        BOOL bCopy5 = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy5)
            goto cleanup;
#endif //!CRASHRPT_LIB
		sFileName.Format(_T("\\CrashSender%d.exe"), CRASHRPT_VER);
        BOOL bCopy2 = CopyFile(sExeFolder+sFileName, sWorkingFolder+sFileName, TRUE);
        if(!bCopy2)
            goto cleanup;

        BOOL bCopy4 = CopyFile(sExeFolder+_T("\\Tests.exe"), sWorkingFolder+_T("\\Tests.exe"), TRUE);
        if(!bCopy4)
            goto cleanup;
        BOOL bCopy9 = CopyFile(sExeFolder+_T("\\crprober.exe"), sWorkingFolder+_T("\\crprober.exe"), TRUE);
        if(!bCopy9)
            goto cleanup;
#endif

        BOOL bCopy3 = CopyFile(sExeFolder+_T("\\crashrpt_lang.ini"), sWorkingFolder+_T("\\crashrpt_lang.ini"), TRUE);
        if(!bCopy3)
            goto cleanup;

        BOOL bCopy6 = CopyFile(sExeFolder+_T("\\dummy.ini"), sWorkingFolder+_T("\\dummy.ini"), TRUE);
        if(!bCopy6)
            goto cleanup;

        BOOL bCopy7 = CopyFile(sExeFolder+_T("\\dummy.log"), sWorkingFolder+_T("\\dummy.log"), TRUE);
        if(!bCopy7)
            goto cleanup;

        BOOL bCopy8 = CopyFile(sExeFolder+_T("\\dbghelp.dll"), sWorkingFolder+_T("\\dbghelp.dll"), TRUE);
        if(!bCopy8)
            goto cleanup;
    }

    /* Create new process */

#ifdef _DEBUG
    sCmdLine = _T("\"") + sWorkingFolder+_T("\\Testsd.exe") + _T("\"");
#else
    sCmdLine = _T("\"") + sWorkingFolder+_T("\\Tests.exe") + _T("\"");
#endif

    if(g_bRunningFromUNICODEFolder)
        sCmdLine += _T(" /unicode");

    {
        HANDLE hProcess = NULL;

        STARTUPINFO si;
        memset(&si, 0, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);

        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));

        BOOL bCreateProcess = CreateProcess(NULL, sCmdLine.GetBuffer(0),
            NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (!bCreateProcess)
        {
            _tprintf(_T("Error creating process! Press any key to exit.\n."));
            _getch();
            goto cleanup;
        }

        hProcess = pi.hProcess;

        // Wait until process exits.
        WaitForSingleObject(hProcess, INFINITE);

        GetExitCodeProcess(hProcess, &dwExitCode);
    }
cleanup:

    // Clean up
    if(g_bRunningFromUNICODEFolder)
        Utility::RecycleFile(sWorkingFolder, TRUE);

    return dwExitCode;
}

//--------------------------------------------------------
// CTestSuite impl
//--------------------------------------------------------

CTestSuite* g_pCurTestSuite = NULL;

CTestSuite::CTestSuite(CTestSuite* pParentSuite)
{
    m_pParentSuite = pParentSuite;
}

std::vector<std::string> CTestSuite::GetTestList(std::set<std::string>& SuitesToRun, bool bIncludeChildren)
{
    std::vector<std::string> test_list;

    if(GetParentSuite()!=NULL && SuitesToRun.size()!=0)
    {
        std::string sSuiteName;
        std::string sSuiteDescription;
        GetSuiteInfo(sSuiteName, sSuiteDescription);

        // Check if this suite's name is in list
        std::set<std::string>::iterator it = SuitesToRun.find(sSuiteName);
        if(it==SuitesToRun.end())
            return test_list; // This suite is not in list
    }

    test_list.push_back("SetUp");

    DoWithMyTests(GET_NAMES, test_list);

    if(bIncludeChildren)
    {
        UINT i;
        for(i=0; i<GetChildSuiteCount(); i++)
        {
            CTestSuite* pChildSuite = GetChildSuite(i);

            std::vector<std::string> child_test_list = pChildSuite->GetTestList(SuitesToRun, bIncludeChildren);

            UINT j;
            for(j=0; j<child_test_list.size(); j++)
            {
                test_list.push_back(child_test_list[j]);
            }
        }
    }

    test_list.push_back("TearDown");

    return test_list;
}

bool CTestSuite::Run(std::set<std::string>& SuitesToRun)
{
    ClearErrors();

    if(GetParentSuite()!=NULL && SuitesToRun.size()!=0)
    {
        std::string sSuiteName;
        std::string sSuiteDescription;
        GetSuiteInfo(sSuiteName, sSuiteDescription);

        // Check if this suite's name is in list
        std::set<std::string>::iterator it = SuitesToRun.find(sSuiteName);
        if(it==SuitesToRun.end())
            return true; // This suite is not in list
    }

    g_pCurTestSuite = this;

    BeforeTest("SetUp");
    SetUp();
    AfterTest("SetUp");

    if(m_bTestFailed)
        m_bSuiteSetUpFailed = true;

    std::vector<std::string> test_list;
    DoWithMyTests(RUN_TESTS, test_list);

    UINT i;
    for(i=0; i<m_apChildSuites.size(); i++)
    {
        m_apChildSuites[i]->Run(SuitesToRun);
    }

    int nErrors = (int)m_asErrorMsg.size();

    g_pCurTestSuite = this;

    if(BeforeTest("TearDown"))
        TearDown();
    AfterTest("TearDown");

    if(nErrors!=(int)m_asErrorMsg.size())
        return false; // TearDown has failed

    g_pCurTestSuite = NULL;

    return m_asErrorMsg.size()==0?true:false;
}

CTestSuite* CTestSuite::GetParentSuite()
{
    return m_pParentSuite;
}

void CTestSuite::SetParentSuite(CTestSuite* pParent)
{
    m_pParentSuite = pParent;
}

UINT CTestSuite::GetChildSuiteCount()
{
    return (UINT)m_apChildSuites.size();
}

void CTestSuite::AddChildSuite(CTestSuite* pChildSuite)
{
    m_apChildSuites.push_back(pChildSuite);
}

CTestSuite* CTestSuite::GetChildSuite(UINT i)
{
    return m_apChildSuites[i];
}

bool CTestSuite::BeforeTest(const char* szFunction)
{
    m_bTestFailed = false;
    std::string sSuiteName;
    std::string sSuiteDescription;
    GetSuiteInfo(sSuiteName, sSuiteDescription);

    printf(" - %s::%s... ", sSuiteName.c_str(), szFunction);

    if(m_bSuiteSetUpFailed)
    {
        m_bTestFailed = true;
        AddErrorMsg(szFunction, "SetUp Failure", NULL);
        return false; // Prevent running test
    }

    return true;
}

void CTestSuite::AfterTest(const char* szFunction)
{
    UNREFERENCED_PARAMETER(szFunction);

    if(!m_bTestFailed)
        printf("OK.\n");
    else
    {
        printf("Failed.\n");

        std::string sErrorMsg = m_asErrorMsg[m_asErrorMsg.size()-1];
        sErrorMsg = "!!! " + sErrorMsg;
        sErrorMsg+="\n";
        printf(sErrorMsg.c_str());
    }
}

//void CTestSuite::AddErrorMsg(const char* szFunction, const char* szAssertion)
//{
//    m_bTestFailed = true;
//
//    std::string sMsg = "In test: ";
//    sMsg += szFunction;
//    sMsg += " Expr: ";
//    sMsg += szAssertion;
//    m_asErrorMsg.push_back(sMsg);
//}

void CTestSuite::AddErrorMsg(const char* szFunction, const char* szAssertion, const char* szMsg, ...)
{
    m_bTestFailed = true;
	char szBuffer[4096];
	memset(szBuffer, 0, 4096);
	if(szMsg!=NULL)
	{
		va_list arg_list;
		va_start(arg_list, szMsg);
		vsnprintf_s(szBuffer, 4095, 4095, szMsg, arg_list);
	}

    std::string sMsg = "In test: ";
    sMsg += szFunction;
    sMsg += " Expr: ";
    sMsg += szAssertion;
	if(szMsg!=NULL)
	{
		sMsg += " Msg: ";
		sMsg += szBuffer;
	}
    m_asErrorMsg.push_back(sMsg);

	std::string sErrorMsg = "!!! " + sMsg;
    sErrorMsg += "\n";
    printf("%s", sErrorMsg.c_str());
    fflush(stdout);
}

std::vector<std::string> CTestSuite::GetErrorList(bool bIncludeChildren)
{
    std::vector<std::string> asErrors = m_asErrorMsg;

    if(bIncludeChildren)
    {
        UINT i;
        for(i=0; i<GetChildSuiteCount(); i++)
        {
            CTestSuite* pChildSuite = GetChildSuite(i);
            std::vector<std::string> asChildErrors = pChildSuite->GetErrorList(bIncludeChildren);

            UINT j;
            for(j=0; j<asChildErrors.size(); j++)
            {
                asErrors.push_back(asChildErrors[j]);
            }
        }
    }

    return asErrors;
}

void CTestSuite::ClearErrors()
{
    m_bSuiteSetUpFailed = false;
    m_bTestFailed = false;
    m_asErrorMsg.clear();
}

//--------------------------------------------------------
// CTestRegistry implementation
//--------------------------------------------------------

CTestRegistry* CTestRegistry::GetRegistry()
{
    static CTestRegistry* pRegistry = NULL;

    if(pRegistry==NULL)
        pRegistry = new CTestRegistry();

    return pRegistry;
}

CTestRegistry::CTestRegistry()
{
    m_pTopSuite = NULL;
}

CTestSuite* CTestRegistry::GetTopSuite()
{
    if(m_pTopSuite==NULL)
        m_pTopSuite = new TopLevelTestSuite();

    return m_pTopSuite;
}
