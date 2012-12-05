//////////////////////////////////////////////////
// PRuleFile.cpp

#include "stdafx.h"

#include "PRuleFile.h"


CPRuleFile::CPRuleFile()
{
	// ��ȡ�����ļ�������·��
	CHAR *p;
	::GetFullPathName(RULE_FILE_NAME, MAX_PATH, m_szPathName, &p);

	m_hFile = INVALID_HANDLE_VALUE;

	// Ϊ���˹���Ԥ�����ڴ�ռ�
	m_nLspMaxCount = 50;

	m_pLspRules = new RULE_ITEM[m_nLspMaxCount];

	m_bLoad = FALSE;
}

CPRuleFile::~CPRuleFile()
{
	if(m_hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFile);	
	delete[] m_pLspRules;
	
}


void CPRuleFile::InitFileData()
{
	// ��ʼ���ļ�ͷ
	strcpy(m_header.szSignature, RULE_HEADER_SIGNATURE);
	m_header.ulHeaderLength = sizeof(m_header);
	
	m_header.ucMajorVer = RULE_HEADER_MAJOR;
	m_header.ucMinorVer = RULE_HEADER_MINOR;
    
	m_header.dwVersion = RULE_HEADER_VERSION;
	strcpy(m_header.szServerIp,"");
	m_header.bisServerAllowed = FALSE;
	strcpy(m_header.szWebURL, RULE_HEADER_WEB_URL); 

	strcpy(m_header.szEmail, RULE_HEADER_EMAIL);

	m_header.ulLspRuleCount = 0;
	

	m_header.ucLspWorkMode = PF_QUERY_ALL;


	m_header.bAutoStart = FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
// ������д��ָ���ļ�
///////////////////////////////////////////////////////////////////////////////////////
BOOL CPRuleFile::WriteRules(CHAR *pszPathName) 
{
	DWORD dw;
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hFile);
	}
	// ���ļ�
	m_hFile = ::CreateFile(pszPathName, GENERIC_WRITE, 
					0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		// д�ļ�ͷ
		::WriteFile(m_hFile, &m_header, sizeof(m_header), &dw, NULL);
		// дӦ�ò���˹���
		if(m_header.ulLspRuleCount > 0)
		{
			::WriteFile(m_hFile, 
						m_pLspRules, m_header.ulLspRuleCount * sizeof(RULE_ITEM), &dw, NULL);
		}
		
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		return TRUE;
	}
	return FALSE;
}

BOOL CPRuleFile::OpenFile()
{
	// ���ȱ�֤�ļ��Ѿ�����
	if(::GetFileAttributes(m_szPathName) == -1)
	{
		InitFileData();
		if(!WriteRules(m_szPathName))
			return FALSE;
	}
	// ���û�йرգ��͹ر�
	if(m_hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFile);

	// ��ֻ����ʽ���ļ�
	m_hFile = ::CreateFile(m_szPathName, GENERIC_READ, 
					FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

	return m_hFile != INVALID_HANDLE_VALUE;
}

///////////////////////////////////////////////////////////////////////////////////////
//  ���ع����ļ�
///////////////////////////////////////////////////////////////////////////////////////
BOOL CPRuleFile::LoadRules()
{
	// �ȴ��ļ�
	if((!OpenFile()) || (::SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN) == -1))
		return FALSE;

	// ���ļ��ж�ȡ����
	DWORD dw = 0;
	do
	{
		// ���ļ�ͷ
		::ReadFile(m_hFile, &m_header, sizeof(m_header), &dw, NULL);
		// ���ǩ������ȷ���˳�
		if( (dw != sizeof(m_header)) || (_tcscmp(m_header.szSignature, RULE_HEADER_SIGNATURE) != 0) )
			break;

		// ��Ӧ�ò���˹���
		if(m_header.ulLspRuleCount > 0)
		{		
			if(m_header.ulLspRuleCount > (ULONG)m_nLspMaxCount)
			{
				m_nLspMaxCount = m_header.ulLspRuleCount;
				delete[] m_pLspRules;
				m_pLspRules = new RULE_ITEM[m_nLspMaxCount];
			}
			if(!::ReadFile(m_hFile, m_pLspRules, 
				m_header.ulLspRuleCount * sizeof(RULE_ITEM), &dw, NULL))
				break;
		}

		

		m_bLoad = TRUE;
	}
	while(FALSE);

	::CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;

	return m_bLoad;
}

BOOL CPRuleFile::SaveRules()
{
	// ���û�м��أ��˳�
	if(!m_bLoad)
		return FALSE;
	// �������
	return WriteRules(m_szPathName);
}

BOOL CPRuleFile::AddLspRules(RULE_ITEM *pItem, int nCount)
{
	if((pItem == NULL) || !m_bLoad)
		return FALSE;

	// ���ȱ�֤���㹻����ڴ�ռ�
	if(m_header.ulLspRuleCount + nCount > (ULONG)m_nLspMaxCount)
	{
		m_nLspMaxCount = 2*(m_header.ulLspRuleCount + nCount);

		RULE_ITEM *pTmp = new RULE_ITEM[m_header.ulLspRuleCount];
		memcpy(pTmp, m_pLspRules, m_header.ulLspRuleCount);

		delete[] m_pLspRules;
		m_pLspRules = new RULE_ITEM[m_nLspMaxCount];
		memcpy(m_pLspRules, pTmp, m_header.ulLspRuleCount);
		delete[] pTmp;
	}
	// ���ӹ���
	memcpy(m_pLspRules + m_header.ulLspRuleCount, pItem, nCount * sizeof(RULE_ITEM));
	m_header.ulLspRuleCount += nCount;
	return TRUE;
}


BOOL CPRuleFile::DelLspRule(int nIndex)
{
	if(((ULONG)nIndex >= m_header.ulLspRuleCount) || !m_bLoad)
		return FALSE;
	// ɾ��һ����Ա
	memcpy(&m_pLspRules[nIndex], 
		&m_pLspRules[nIndex + 1], (m_header.ulLspRuleCount - nIndex) * sizeof(RULE_ITEM));

	m_header.ulLspRuleCount --;
	return TRUE;
}