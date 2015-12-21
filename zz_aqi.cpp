#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <time.h>
#include <curl/curl.h>

#include <string>
#include <list>
#include <sstream>
#include <iostream>

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
			curl_easy_setopt(_curl,CURLOPT_VERBOSE,&on);
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

static void test(){
	DbConf db;
	db.load_db_conf("db.xml");
	std::cout<<"host:"<<db.get_db_host()<<std::endl;
	std::cout<<"user:"<<db.get_db_user()<<std::endl;
	std::cout<<"name:"<<db.get_db_name()<<std::endl;
	std::cout<<"passwd:"<<db.get_db_passwd()<<std::endl;
}

int main(int argc,char **argv){
	test();
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

