//文件：teach.cpp
//本程序是一个简单的从数据文件中读取并执行问题的问答执行系统，数据文件包含了固定格式的全部课程问题，读取问题并处理问题存入程序中特有格式的数据结构中，然后执行问答，数据文件的格式模板在附件中，标准格式的文字规定在课程报告中，内部数据结构在一下程序中
#include<iostream>
#include<fstream>
#include<string>
#include<stdlib.h>
using namespace std;

//常数
//-----
//#define MaxQuestions 100           -- 一个课程数据库最大问题数
//#define MaxLinesPerQuestion  100   -- 每个问题最大行数
//#define MaxAnswersPerQuestion   10 -- 每个答案最大行数
//#define EndMarker "-----"          -- 问题文本结束符
#define MaxQuestions 100
#define MaxLinesPerQuestion 100
#define MaxAnswersPerQuestion 10
#define EndMarker "-----"

//数据结构
//类型：answerT
//--------------
//这种结构向每个问题可能的答案提供空间
struct answerT
{
	string ans; //答案
	int nextq;  //下一题
};

//数据结构
//类型：questionT
//----------------
//这种结构向课程数据库中的每一个问题提供空间
struct questionT
{
	int qnumber;  //题号（新增加的，主要是防止数据文件中的题重复）
	string qtext[MaxLinesPerQuestion];      //问题文本
	answerT *answers[MaxAnswersPerQuestion];//问题答案
	int nAnswers; //有效的答案数组长度
};

//数据结构
//类型：courseDB
//---------------
//这种结构向整个课程数据库提供空间
struct courseDB
{
	string title;  //课程数据库名称
	questionT *questions[MaxQuestions]; //所有题目
	int nQuestion; //有效的题目数组长度
};

//数据结构
//类型：Error
//------------
//这种结构向用户提交的数据文件每个可能出现的错误的文本信息提供空间
struct Error
{
	int qnumber; //题号
	string Etext;//错误信息
};

//函数：OpenFile
//用法：infile = OpenFile();
//---------------------------
//此函数将用户输入的目录文件打开并将文件流返回
ifstream OpenFile();

//函数：CkeckDatabase
//用法：if (CkeckDatabase(infile)){...}
//--------------------------------------
//此函数用于校验数据文件，按照规定的格式进行校验，如果格式没有按照模板文件来，那么就会指出相应的错误，并退出程序
bool CkeckDatabase(ifstream &infile);

//函数：intToString
//用法：str=intTostring(10);
//--------------------------
//将数字转换成字符串
string intToString(int i);

//函数：ReadDatabaseAndProcessCourse
//用法：db=ReadDatabaseAndProcessCourse(infile);
//----------------------------------------------
//此程序将从数据文件读取的文件流里的数据按照特定的格式进行解析，存放在内部数据结构中，最开始读取题号，然后读取题目文本，遇到结束符开始读取答案，遇到空行又读取题号，如此循环，直到到达文件尾。
courseDB *ReadDatabaseAndProcessCourse(ifstream &infile);

//函数：AskQuestion
//用法：AskQuestion(db);
//----------------------------------------------
//从第一题开始提问，然后用户输入答案，如果查不到对应的答案，则重复问题，查到相应的答案就转向答案所对应的题目
void AskQuestion(courseDB *db);


//主函数
int main()
{
	//读取的文件流
	ifstream infile;
	//处理后的课程数据库
	courseDB *db = new courseDB();
	//读取数据文件
	infile = OpenFile();

	//判断是否读取成功
	if (!infile.is_open())
	{
		cout << "打开数据文件失败，请查看文件路径是否正确或者文件是否在别的程序中打开，然后重试！" << endl;
		return 0;
	}

	cout << "文件读取成功！" << endl << endl << "正在校验文件..." << endl;

	//校验数据文件
	if (CkeckDatabase(infile))
		cout << "数据文件校验成功！" << endl << endl;
	else
	{
		//关闭文件流
		infile.close();
		cout << "数据文件校验失败，请将错误修改后重试！" << endl << endl;
		cout << "再见" << endl;
		system("pause");
		return 0;
	}

	//将文件流指针移回文件头
	infile.clear();
	infile.seekg(0, ios::beg);

	cout << "正在处理文件..." << endl;
	//读取数据文件并处理课程数据库
	db = ReadDatabaseAndProcessCourse(infile);
	cout << "文件处理成功！" << endl << "开始答题！" << endl << endl;

	//提问
	AskQuestion(db);

	//关闭文件流
	infile.close();
	cout << "再见" << endl;

	system("pause");
	return 0;
}

//读取数据文件
ifstream OpenFile()
{
	ifstream openFile;
	char *filename = new char;

	//输入文件路径
	cout << "请输入数据文件路径(如：C:\\\\Users\\\\Administrator\\\\Desktop\\\\test.txt)：" << endl;
	cin >> filename;

	cout << endl << "正在读取文件..." << endl;
	//尝试打开数据文件并返回
	openFile.open(filename);
	return openFile;
}

//检验数据文件的函数
bool CkeckDatabase(ifstream &infile)
{
	//用于读取一行数据的数组
	char buffer[256];
	//用于存放题号的数组
	int qNumber[MaxQuestions];
	//用于存放答案转向的题号
	int aQnumber[MaxQuestions + 1];
	//记录当前循环应该读取的内容
	bool readQnumber = true, readQtext = false, readAnswer = false;
	//记录错误
	Error *error[MaxQuestions];
	//错误的实际条数
	int eNumber = 0;
	//题目行数记录
	int qLineNumber = 0;
	//答案行数记录
	int aLineNumber = 0;
	//题号实际数
	int _qNumber = 0;
	//答案中题号的实际数
	int _aQnumber = 0;
	//记录是否已经添加了问题行数超出限制的错误
	bool ReqLineError = false;
	//记录是否已经添加了答案行数超出限制的错误
	bool ReaLineError = false;

	//校验文件时标题项不要，直接去掉
	if (!infile.eof())
		infile.getline(buffer, 100);

	//开始校验
	while (!infile.eof())
	{
		//问题超过100个，退出
		if (_qNumber >= MaxQuestions)
		{
			cout << "数据文件中的题目超过规定的题目数量：" << intToString(MaxQuestions) << "道，请修改后再试!!!" << endl;
			return false;
		}

		//答案中的题号超过101个，退出
		if (_aQnumber >= MaxQuestions + 1)
		{
			cout << "数据文件中的答案转向的题号超过规定一个数据文件规定的题目数量：" << MaxQuestions << "道，请修改后再试!!!" << endl;
			return false;
		}

		//一行一行读取,并将字符数组转化为字符串
		infile.getline(buffer, 100);
		string _buffer(&buffer[0], &buffer[strlen(buffer)]);

		//读取题号
		if (readQnumber)
		{
			//将题号转化为数字，要先将string转化为char*类型，才能作为参数传给函数atoi，此函数头文件stdlib.h中
			qNumber[_qNumber] = atoi(_buffer.c_str());

			_qNumber++;

			//读取完成后要将记录重新设置
			readQnumber = readAnswer = false;
			readQtext = true;
			continue;
		}

		//读取问题文本
		if (readQtext)
		{
			if (_buffer == EndMarker)
			{
				//如果遇到结束符，那么直接设置记录后结束本次循环进入下一次循环
				readQnumber = readQtext = false;
				readAnswer = true;
				//将题目行数清零
				qLineNumber = 0;
				//还原题目错误记录为：没有记录错误
				ReqLineError = false;
				continue;
			}

			//题目行数超出100行时记录错误并且没有记录过本次错误就进入, 避免重复记录错误
			if (qLineNumber >= MaxLinesPerQuestion && !ReqLineError)
			{
				//查找题号是否存在，存在则直接将错误添加到该题号所对应的内容记录中，这里错误条数记录不用增加

				//记录是否已经记录了该错误
				bool flag = false;
				for (int i = 0; i < eNumber; i++)
				{
					//因为记录了题号之后会随即加一，所以此处必须减一，否则数组会出界
					if (error[i]->qnumber == qNumber[_qNumber - 1])
					{
						error[eNumber]->Etext += ", 题目行数超出规定行数：" + intToString(MaxLinesPerQuestion);
						flag = true;
						//设置记录，表示已经记录过本次错误
						ReqLineError = true;
						break;
					}
				}

				//如果已经记录了，那么就直接结束本次循环
				if (flag)
					continue;

				//记录错误、结束本次循环并将错误条数记录加一
				error[eNumber] = new Error;
				error[eNumber]->qnumber = qNumber[_qNumber - 1];//因为记录了题号之后会随即加一，所以此处必须减一，否则数组会出界
				error[eNumber]->Etext = "题目行数超出规定行数：" + intToString(MaxLinesPerQuestion);
				eNumber++;
				//设置记录，表示已经记录过本次错误
				ReqLineError = true;
				continue;
			}

			//记录行数
			qLineNumber++;
			continue;
		}

		//读取答案
		if (readAnswer)
		{
			//如果等于换行，那么就要结束本次循环，开始进入下一题的记录了
			if (_buffer == "\r\n" || _buffer == "")
			{
				//设置标志记录
				readAnswer = readQtext = false;
				readQnumber = true;

				//答案行数清零
				aLineNumber = 0;
				ReaLineError = false;
			}

			//如果答案的行数超出规定的行数并且没有记录过该错误，那么就进入, 避免重复记录错误
			if (aLineNumber >= MaxAnswersPerQuestion && !ReaLineError)
			{
				//查找题号是否存在，存在则直接将错误添加到该题号所对应的内容记录中，这里错误条数记录不用增加

				//记录是否已经记录了该错误
				bool flag = false;
				for (int i = 0; i < eNumber; i++)
				{
					//因为记录了题号之后会随即加一，所以此处必须减一，否则数组会出界
					if (error[i]->qnumber == qNumber[_qNumber - 1])
					{
						error[eNumber]->Etext += ", 答案行数超出规定行数：" + intToString(MaxAnswersPerQuestion);
						flag = true;
						//设置记录，表示已经记录过本次错误
						ReaLineError = true;
						break;
					}
				}

				//如果已经记录了，那么就直接结束本次循环
				if (flag)
					continue;

				//记录错误、结束本次循环并将错误条数记录加一
				error[eNumber] = new Error;
				error[eNumber]->qnumber = qNumber[_qNumber - 1];//因为记录了题号之后会随即加一，所以此处必须减一，否则数组会出界
				error[eNumber]->Etext = "答案行数超出规定行数：" + intToString(MaxAnswersPerQuestion);
				eNumber++;
				//设置记录，表示已经记录过本次错误
				ReaLineError = true;
				continue;
			}

			aLineNumber++;

			//分析答案中的题号
			//将：后面的数字取出来转化为数字
			//先找到：的位置，然后分割字符串，取得：后面的字符串，转化为char*的格式，再传给函数atoi转化为数字
			int number = atoi(_buffer.substr(_buffer.find_first_of(":") + 1).c_str());

			//查找是否已经存在该题号
			//标杆
			bool flag = false;
			for (int i = 0; i < _aQnumber; i++)
			{
				if (aQnumber[i] == number)
				{
					flag = true;
					break;
				}
			}

			//已记录，就跳出本次循环
			if (flag)
				continue;

			aQnumber[_aQnumber] = number;
			_aQnumber++;
		}
	}

	//标杆，查看是否有错误
	int _flag = 0;
	if (eNumber != 0)
	{
		_flag = 1;
		for (int i = 0; i < eNumber; i++)
		{
			cout << "第" << error[i]->qnumber << "题,错误: " + error[i]->Etext << endl;
		}
	}

	//标杆
	bool flag = false;
	for (int i = 0; i < _aQnumber; i++)
	{
		//标杆
		flag = false;
		for (int j = 0; j < _qNumber; j++)
		{
			if (qNumber[j] == aQnumber[i] || aQnumber[i] == 0)
			{
				flag = true;
				break;
			}
		}
		if (!flag)
		{
			cout << "题号错误：答案中的转向的题号:" << aQnumber[i] << " 在问题的题号中找不到，且不为0！" << endl;
			_flag = 1;
		}
	}

	//记录，让提示语不重复
	int Rec[100];
	int _rec = 0;
	bool isRec = false;
	for (int i = 0; i < _qNumber; i++)
	{
		//记录该题号重复几次
		int num = 0;
		//记录是否重复
		isRec = false;

		//开始查找是否重复
		for (int j = 0; j < _qNumber; j++)
		{
			if (qNumber[i] == qNumber[j])
				num++;
		}

		//开始查找是否已经记录该题号
		for (int k = 0; k < _rec; k++)
		{
			if (Rec[k] == qNumber[i])
				isRec = true;
		}

		//如果没有提示该题重复或者是循环的第一次，则让进入
		if (isRec == false || _rec == 0)
		{
			//记录该题号，表示已经处理过了
			Rec[_rec] = qNumber[i];
			_rec++;

			//若重复，则提示该题号重复
			if (num > 1)
			{
				cout << "题号错误：题号:" << qNumber[i] << " 重复" << num - 1 << "次" << endl;
				_flag = 1;
			}
		}
	}

	if (_flag == 1)
		return false;
	else
		return true;
}

//读取数据文件并处理课程数据库
//因为前面进行了文件校验，所以这里直接进行读取并处理就可以了
courseDB *ReadDatabaseAndProcessCourse(ifstream &infile)
{
	//实例一个课程数据库
	courseDB *db = new courseDB();
	db->nQuestion = 0;
	//声明一个用于接收一行数据的数组
	char buffer[256];
	//记录当前循环应该读取的内容
	bool readQnumber = true, readQtext = false, readAnswer = false;
	//题目行数记录
	int qLineNumber = 0;
	//答案行数记录
	int aLineNumber = 0;


	//首先读取课程的标题
	if (!infile.eof())
	{
		infile.getline(buffer, 100);
		string _buffer(&buffer[0], &buffer[strlen(buffer)]);
		db->title = _buffer;
	}
	while (!infile.eof())
	{
		//读取数据并处理成字符串格式
		infile.getline(buffer, 100);
		string _buffer(&buffer[0], &buffer[strlen(buffer)]);

		//读取题号
		if (readQnumber)
		{
			//处理题号
			db->questions[db->nQuestion] = new questionT();
			db->questions[db->nQuestion]->qnumber = atoi(_buffer.c_str());
			db->nQuestion++;

			//读取完成后改变标志记录并结束本次循环
			readAnswer = readQnumber = false;
			readQtext = true;
			continue;
		}

		//读取问题文本
		if (readQtext)
		{
			if (_buffer == EndMarker)
			{
				//遇到结束标志设置标志记录并结束本次循环
				readQnumber = readQtext = false;
				readAnswer = true;
				qLineNumber = 0;
				continue;
			}

			//因为前面处理完题号后加一，所以现在需要减一，因为数组是从0开始，而题目个数是从1开始的。
			db->questions[db->nQuestion - 1]->qtext[qLineNumber] = _buffer;
			qLineNumber++;
			continue;
		}

		//读取答案
		if (readAnswer)
		{
			if (_buffer == "\r\n" || _buffer == "")
			{
				//遇到空行后就改变标志记录然后退出本次循环
				readAnswer = readQtext = false;
				readQnumber = true;
				//设置答案行数
				db->questions[db->nQuestion - 1]->nAnswers = aLineNumber;
				aLineNumber = 0;
				continue;
			}

			//因为前面处理完题号后加一，所以现在需要减一
			db->questions[db->nQuestion - 1]->answers[aLineNumber] = new answerT();

			//处理答案内容和转向的题目
			db->questions[db->nQuestion - 1]->answers[aLineNumber]->ans = _buffer.substr(0, _buffer.find_first_of(":"));
			db->questions[db->nQuestion - 1]->answers[aLineNumber]->nextq = atoi(_buffer.substr(_buffer.find_first_of(":") + 1).c_str());

			aLineNumber++;
			continue;
		}

	}
	return db;
}

//提问
void AskQuestion(courseDB *db)
{
	//当前提问的问题
	questionT *question = new questionT();
	//下一题的题号
	int nextq = 1;
	//用户输入的内容
	string readText;

	//输出题库标题
	cout << db->title << endl;

	//开始提问
	while (nextq != 0)
	{
		//找到下一题
		for (int i = 0; i < db->nQuestion; i++)
		{
			if (db->questions[i]->qnumber == nextq)
			{
				question = db->questions[i];
				break;
			}
		}

		//提问
		for (int i = 0; question->qtext[i] != ""; i++)
		{
			cout << question->qtext[i] << endl;
		}

		//用户输入答案
		cin >> readText;

		//校验答案
		//标杆
		bool flag = false;
		for (int i = 0; i < question->nAnswers; i++)
		{
			if (question->answers[i]->ans == readText)
			{
				nextq = question->answers[i]->nextq;
				flag = true;
				break;
			}
		}

		//如果找到对应的答案则结束本次循环
		if (flag)
		{
			cout << endl;
			continue;
		}

		cout << "所选答案与给出的答案不符，请重新作答！" << endl << endl;
	}

}

//将数字转换成字符串
string intToString(int i)
{
	//将数字转化为字符数组，然后再转化为字符串
	char str[10];
	sprintf_s(str, "%d", i);
	string _str(&str[0], &str[strlen(str)]);
	return _str;
}