#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <time.h>
#include <curl/curl.h>
#include <rapidjson/document.h>

#include <string>
#include <list>
#include <sstream>
#include <iostream>

#include <mysql++.h>

/// @brief 站点类
class Station{
	public:
		Station(){}
		Station(const std::string& name,const std::string &id)
			:_name(name),_id(id)
		{}

		std::string& get_name(){
			return _name;
		}
		const std::string& get_name()const {
			return _name;
		}
		std::string& get_id(){
			return _id;
		}
		const std::string& get_id()const{
			return _id;
		}

	private:
		std::string _name;   ///< 站点名称
		std::string _id;     ///< 站点id，构造url使用
};

/// @brief 站点集合
class StationSet{
	public:
		/// @brief 依据xml文件内容载入站点信息
		void load_stations(const std::string &path);
		/// @brief 输出站点信息到FILE
		void output(FILE *fp=stdout);
		std::list<Station> _stations;         ///< 站点链表
	private:
		void parse_station_node(xmlDocPtr doc,xmlNodePtr st);
};

/// @brief 封装获取aqi数据的操作
class AQIUrl{
	public:
		AQIUrl(){
			_curl=NULL;
			_curl=curl_easy_init();
			long on=1;
			//curl_easy_setopt(_curl,CURLOPT_VERBOSE,&on);
		}
		~AQIUrl(){
			if(NULL!=_curl)
				curl_easy_cleanup(_curl);
		}
		/// @breif 获取aqi url字符串
		std::string get_aqi_url(const std::string &id,struct tm & time);
		std::string get_aqi_url(const std::string &id);
		std::string get_aqi_url(const Station &st);

		/// @brief 请求url并返回内容
		bool get_aqi_data(const std::string &url,std::string &aqi_data);
		bool get_aqi_data(const Station &st,std::string &aqi_data);
	private:
		static size_t write_function(char *buf,size_t size,size_t nmemb,void *data){
			std::string *str=(std::string *)data;
			*str+=std::string(buf,size*nmemb);
			return size*nmemb;
		}
	private:
		CURL *_curl;
};


class DbConf{
	public:
		DbConf(){
			init();
		}

		/// @brief 读取数据库配置文件
		bool load_db_conf(const std::string &filepath);

		const std::string& get_db_host(){
			return _host;
		}
		const std::string& get_db_name(){
			return _name;
		}
		const std::string& get_db_user(){
			return _user;
		}
		const std::string& get_db_passwd(){
			return _passwd;
		}
	private:
		void init();
	private:
		std::string _host;   ///< 数据库主机
		std::string _name;   ///< 数据库名
		std::string _user;   ///< 登陆用户
		std::string _passwd; ///< 登陆密码
};

struct AQIUint{
	time_t time;
	unsigned int aqi;
	double pm25;
	unsigned int pm25_aqi;
	double pm10;
	unsigned int pm10_aqi;
	double o3;
	unsigned int o3_aqi;
	double co;
	unsigned int co_aqi;
	double so2;
	unsigned int so2_aqi;
	double no2;
	unsigned int no2_aqi;

	friend std::ostream & operator << (std::ostream &os,const AQIUint &uint);

	std::string gen_sql(const std::string &id)const ;

	private:
		std::string time_to_string(time_t t) const;

};

class AQIJsonParser {
	public:
		bool parse(const std::string &str,AQIUint &uint);
	private:
		time_t get_time(const std::string &str);
};

class AQIWriter {
	public:
		AQIWriter(const std::string &confpath);
		void writer(const AQIUint &uint,const std::string &id);
		AQIWriter(AQIWriter &w)=delete;
		AQIWriter & operator =(AQIWriter &w)=delete;
	private:
		mysqlpp::Connection _conn;
		DbConf _dbconf;
};

/// @brief 获取aqi数据并写入数据库
class RecorderAQI {
	public:
		RecorderAQI(){}
		void load_station_set(const std::string &path);
		void record(AQIWriter &wirter);
		RecorderAQI(const RecorderAQI &re)=delete;
		RecorderAQI& operator =(RecorderAQI &re)=delete;
	private:
		StationSet _set;
		AQIUrl _url;
		AQIJsonParser _parser;
};

static void test(){
	StationSet set;
	set.load_stations("stations.xml");
	AQIUrl url;
	for(auto it=set._stations.begin();it!=set._stations.end();++it){
		std::string json;
		url.get_aqi_data(*it,json);
		json.push_back('\0');
		AQIUint uint;
		AQIJsonParser parser;
		parser.parse(json,uint);
		std::cout<<"name:"<<it->get_name()<<"\n";
		std::cout<<uint<<"\n";
		std::cout<<"sql:\n";
		std::cout<<uint.gen_sql(it->get_id());
		std::cout<<"\n=========================\n";
	}
}

int main(int argc,char **argv){
	RecorderAQI re;
	re.load_station_set("stations.xml");
	AQIWriter writer("db.xml");
	re.record(writer);
}

std::ostream & operator << (std::ostream &os,const AQIUint &uint){
	struct tm *t=localtime(&uint.time);
	os<<t->tm_year+1900<<"-"<<t->tm_mon+1<<"-"<<t->tm_mday<<" "
		<<t->tm_hour<<":"<<t->tm_min<<":"<<t->tm_sec<<"\n";
	os<<"AQI:"<<uint.aqi<<"\n";
	os<<"pm25:"<<uint.pm25<<"-"<<uint.pm25_aqi<<" ";
	os<<"pm10:"<<uint.pm10<<"-"<<uint.pm10_aqi<<" ";
	os<<"o3:"<<uint.o3<<"-"<<uint.o3_aqi<<" ";
	os<<"co:"<<uint.co<<"-"<<uint.co_aqi<<" ";
	os<<"so2:"<<uint.so2<<"-"<<uint.so2_aqi<<" ";
	os<<"no2:"<<uint.no2<<"-"<<uint.no2_aqi<<" ";
}

//////////////RecoderAQI类实现////////////
void RecorderAQI::load_station_set(const std::string &path){
	_set.load_stations(path);
}

void RecorderAQI::record(AQIWriter &writer){
	for(auto it=_set._stations.begin();it!=_set._stations.end();++it){
		std::string json;
		this->_url.get_aqi_data(*it,json);
		json.push_back('\0');
		AQIUint uint;
		this->_parser.parse(json,uint);
		writer.writer(uint,it->get_id());
	}
}

//////////////AQIWrtier类实现///////////////
AQIWriter::AQIWriter(const std::string &confpath) {
	_dbconf.load_db_conf(confpath);
	_conn.connect(_dbconf.get_db_name().c_str(),
				_dbconf.get_db_host().c_str(),
				_dbconf.get_db_user().c_str(),
				_dbconf.get_db_passwd().c_str());
}

void AQIWriter::writer(const AQIUint &uint,const std::string & id){
	mysqlpp::Query query=_conn.query(uint.gen_sql(id).c_str());
	query.execute();
}

//////////////AQIUint类实现//////////////////
std::string AQIUint::time_to_string(time_t t) const {
	struct tm* tt=localtime(&t);
	std::ostringstream sout;
	sout<<tt->tm_year+1900<<"-"
		<<tt->tm_mon+1<<"-"
		<<tt->tm_mday<<" "
		<<tt->tm_hour<<":"
		<<tt->tm_min<<":"
		<<tt->tm_sec;
	return sout.str();
}

std::string AQIUint::gen_sql(const std::string& id) const {
	std::string str="insert into aqi_data values(";
	str+="'"+id+"',";
	str+="'"+time_to_string(time)+"',";
	str+=std::to_string(o3)+","+std::to_string(o3_aqi)+",";
	str+=std::to_string(co)+","+std::to_string(co_aqi)+",";
	str+=std::to_string(so2)+","+std::to_string(so2_aqi)+",";
	str+=std::to_string(no2)+","+std::to_string(so2_aqi)+",";
	str+=std::to_string(pm10)+","+std::to_string(pm10_aqi)+",";
	str+=std::to_string(pm25)+","+std::to_string(pm25_aqi)+",";
	str+=std::to_string(aqi)+");";
	return str;
}

//////////////AQIJsonParser类实现/////////////
bool AQIJsonParser::parse(const std::string &json,AQIUint &uint){
	using namespace rapidjson;
	memset(&uint,0,sizeof(uint));
	Document d;
	d.Parse(json.c_str());
	Value::ConstMemberIterator it=d.FindMember("Head");
	if(it==d.MemberEnd())
		return false;
	const Value &a=it->value;
	if(!a.IsArray()||a.Empty())
		return false;
	const Value &data=a[0u];
	if((it=data.FindMember("CREATE_DATE"))==data.MemberEnd())
		return false;
	auto t=get_time(it->value.GetString());
	if(t==-1)
		return false;
	uint.time=t;
	if((it=data.FindMember("AQI"))==data.MemberEnd())
		return false;
	uint.aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("CO"))!=data.MemberEnd())
		uint.co=std::stod(it->value.GetString());
	if((it=data.FindMember("COIAQI"))!=data.MemberEnd())
		uint.co_aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("NO2"))!=data.MemberEnd())
		uint.no2=std::stod(it->value.GetString());
	if((it=data.FindMember("NO2IAQI"))!=data.MemberEnd())
		uint.no2_aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("O3"))!=data.MemberEnd())
		uint.o3=std::stod(it->value.GetString());
	if((it=data.FindMember("O3IAQI"))!=data.MemberEnd())
		uint.o3_aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("PM10"))!=data.MemberEnd())
		uint.pm10=std::stod(it->value.GetString());
	if((it=data.FindMember("PM10IAQI"))!=data.MemberEnd())
		uint.pm10_aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("PM25"))!=data.MemberEnd())
		uint.pm25=std::stod(it->value.GetString());
	if((it=data.FindMember("PM25IAQI"))!=data.MemberEnd())
		uint.pm25_aqi=std::stoi(it->value.GetString());
	if((it=data.FindMember("SO2"))!=data.MemberEnd())
		uint.so2=std::stod(it->value.GetString());
	if((it=data.FindMember("SO2IAQI"))!=data.MemberEnd())
		uint.so2_aqi=std::stoi(it->value.GetString());
	return true;
}

time_t AQIJsonParser::get_time(const std::string &str){
	std::string temp=str;
	for(auto it=temp.begin();it!=temp.end();++it){
		if(*it=='-'||*it==':')
			*it=' ';
	}
	struct tm t;
	memset(&t,0,sizeof(t));
	std::istringstream sin(temp);
	sin>>t.tm_year>>t.tm_mon>>t.tm_mday
		>>t.tm_hour>>t.tm_min>>t.tm_sec;
	t.tm_year-=1900;
	t.tm_mon-=1;
	return mktime(&t);
}

//////////////DbConf类实现////////////////////

void DbConf::init(){
	_host="localhost";
	_name="aqi";
	_user="hzy";
}

/// @brief 读取数据库配置文件
bool DbConf::load_db_conf(const std::string &filepath){
	xmlDocPtr doc;
	doc=xmlParseFile(filepath.c_str());
	if(nullptr==doc)
		return false;
	xmlNodePtr root=xmlDocGetRootElement(doc);
	if(nullptr==root){
		xmlFreeDoc(doc);
		return false;
	}
	xmlNodePtr node;
	for(node=root->children;node!=NULL;node=node->next){
		if(node->type!=XML_ELEMENT_NODE)
			continue;
		if(xmlStrcmp(node->name,BAD_CAST"user")==0){
			xmlChar* user=xmlNodeListGetString(doc,node->children,1);
			if(nullptr!=user){
				_user=(char*)user;
				xmlFree(user);
			}
		}else if(xmlStrcmp(node->name,BAD_CAST"name")==0){
			xmlChar* name=xmlNodeListGetString(doc,node->children,1);
			if(nullptr!=name){
				_name=(char*)name;
				xmlFree(name);
			}
		}else if(xmlStrcmp(node->name,BAD_CAST"host")==0){
			xmlChar* host=xmlNodeListGetString(doc,node->children,1);
			if(nullptr!=host){
				_host=(char*)host;
				xmlFree(host);
			}
		}else if(xmlStrcmp(node->name,BAD_CAST"passwd")==0){
			xmlChar* passwd=xmlNodeListGetString(doc,node->children,1);
			if(nullptr!=passwd){
				_passwd=(char*)passwd;
				xmlFree(passwd);
			}
		}
	}
	xmlFreeDoc(doc);
	return true;
}

//////////////AQIUrl类实现//////////////////////

/// @breif 获取aqi url字符串
std::string AQIUrl::get_aqi_url(const std::string &id,struct tm & time){
	std::ostringstream sout;
	sout<<"http://www.zzemc.cn/em_aw/Services/DataCenter.aspx"
		<<"?type=getPointHourData"
		<<"&code="<<id
		<<"&time="
		<<time.tm_year+1900<<"-"<<time.tm_mon+1<<"-"<<time.tm_mday
		<<"%20"<<time.tm_hour<<":"<<"00:00";
	return sout.str();
}

/// @breif 获取aqi url字符串
std::string AQIUrl::get_aqi_url(const std::string &id){
	time_t t=time(NULL);
	struct tm *tt=localtime(&t);
	return get_aqi_url(id,*tt);
}

/// @breif 获取aqi url字符串
std::string AQIUrl::get_aqi_url(const Station &st){
	return get_aqi_url(st.get_id());
}

/// @brief 请求url并返回内容
bool AQIUrl::get_aqi_data(const std::string &url,std::string &aqi_data){
	aqi_data.clear();
	curl_easy_setopt(_curl,CURLOPT_URL,url.c_str());
	curl_easy_setopt(_curl,CURLOPT_WRITEFUNCTION,AQIUrl::write_function);
	curl_easy_setopt(_curl,CURLOPT_WRITEDATA,&aqi_data);
	CURLcode code=curl_easy_perform(_curl);
	if(code!=0)
		return false;
	long res;
	curl_easy_getinfo(_curl,CURLINFO_RESPONSE_CODE,&code);
	if(res>=200&&res<300)
		return true;
	return false;
}

/// @brief 请求url并返回内容
bool AQIUrl::get_aqi_data(const Station &st,std::string &aqi_data){
	return get_aqi_data(get_aqi_url(st.get_id()),aqi_data);
}


//////////StationSet类实现/////////////
void StationSet::parse_station_node(xmlDocPtr doc,xmlNodePtr st){
	if(NULL==st)
		return;
	xmlNodePtr child=st->children;
	Station s;
	bool set_name=false;
	bool set_id=false;
	for(;child!=NULL;child=child->next){
		if(child->type!=XML_ELEMENT_NODE)
			continue;
		if(xmlStrcmp(child->name,BAD_CAST"name")==0){
			xmlChar *str=xmlNodeListGetString(doc,child->children,1);
			if(NULL!=str) {
				s.get_name()=std::string((char*)str);
				xmlFree(str);
				set_name=true;
			}
		}
		if(xmlStrcmp(child->name,BAD_CAST"id")==0){
			xmlChar *str=xmlNodeListGetString(doc,child->children,1);
			if(NULL!=str) {
				s.get_id()=std::string((char*)str);
				xmlFree(str);
				set_id=true;
			}
		}
	}
	if(set_name&&set_id){
		_stations.push_back(std::move(s));
	}
}

/// @brief 依据xml文件内容载入站点信息
void StationSet::load_stations(const std::string &path){
	_stations.clear();
	xmlDocPtr doc;
	doc=xmlParseFile(path.c_str());
	if(nullptr==doc)
		return;
	xmlNodePtr root=xmlDocGetRootElement(doc);
	if(nullptr==root){
		xmlFreeDoc(doc);
		return;
	}
	xmlNodePtr st;
	for(st=root->children;st!=NULL;st=st->next){
		if(st->type==XML_ELEMENT_NODE&&xmlStrcmp(st->name,BAD_CAST"station")==0){
			parse_station_node(doc,st);
		}
	}
	xmlFreeDoc(doc);
}

/// @brief 输出站点信息到FILE
void StationSet::output(FILE *fp){
	if(NULL==fp)
		return;
	for(auto it=_stations.begin();it!=_stations.end();++it){
		fprintf(fp,"%s %s\n",it->get_id().c_str(),it->get_name().c_str());
	}
}

