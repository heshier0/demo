#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "zbar.h"

extern "C" {
#include "QRcode_Detection.h"
}


using namespace std;
using namespace zbar;

static void SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

static void split(char *src, const char *separator, char **dest, int *num)
{

	char *pNext;
	int count = 0;
	if (src == NULL || strlen(src) == 0)
		return;
	if (separator == NULL || strlen(separator) == 0)
		return;
	pNext = (char *)strtok(src, separator);
	while (pNext != NULL) {
		*dest++ = pNext;
		++count;
		pNext = (char *)strtok(NULL, separator);
	}
	*num = count;
}


int QRcode_Detection(const char* buffer, int inwidth, int inheight, void** output)
{
	ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	unsigned char *raw = (unsigned char *)buffer;

	Image imageZbar(inwidth, inheight, "Y800", raw, inwidth * inheight);
	scanner.scan(imageZbar);
	Image::SymbolIterator symbol = imageZbar.symbol_begin();

	if (imageZbar.symbol_begin() == imageZbar.symbol_end())
	{
		return -1;
	}
	//output=.c_str()
	*output = (void*)calloc(1024, 1);
	if(NULL == *output)
	{
		return -1;
	}

	for (; symbol != imageZbar.symbol_end(); ++symbol)
	{
		//string tmp = symbol->get_data();
		// output[0]=symbol->get_data().c_str();
		// cout<<output[0]<<endl;
		memcpy(*output, symbol->get_data().c_str(), symbol->get_data().length());

		//cout << "QRcode:" << endl << symbol->get_data() << endl << endl;
		/*
		vector<string> Stringlist1;
		string tempsplit1 = "\"ssid\":\"";
		SplitString(symbol->get_data(), Stringlist1, tempsplit1);
		vector<string> Stringlist2;
		string tempsplit2 = "\",\"bssid\":\"";
		if (Stringlist1.size() > 1)
		{
			SplitString(Stringlist1[1], Stringlist2, tempsplit2);
			if (Stringlist2.size() > 1)
			{
				//wifiname
				output.push_back(Stringlist2[0]);
				vector<string> Stringlist3;
				string tempsplit3 = "\",\"pwd\":\"";
				SplitString(Stringlist2[1], Stringlist3, tempsplit3);
				if (Stringlist3.size() > 1)
				{
					vector<string> Stringlist4;
					string tempsplit4 = "\",\"checkCode\":\"";
					SplitString(Stringlist3[1], Stringlist4, tempsplit4);
					//password
					output.push_back(Stringlist4[0]);

				}
			}
			output.push_back(symbol->get_data());
		}
		else
		{
			output.push_back(symbol->get_data());
		}*/
		

	}

	imageZbar.set_data(NULL, 0);
	return 0;
}
