//
// Created by Addison on 2023/2/3.
//
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <simdjson.h>
#include <iostream>
#include <memory>
#include <fstream>

#include "Config.h"

using namespace boost;
using namespace boost::filesystem;
using namespace std;
using namespace simdjson;

string CONFIG_TPL = R"(
{
    "endpoint": "%s",
    "keyId": "%s",
    "keySecret": "%s",
    "bucketName":"%s"
}
)";

string generate_download_url(string endpoint,string bucket_name){
    string str;
    if(starts_with(endpoint,"https://")) {
        auto url = endpoint.substr(8, endpoint.length());
        str = "https://" + bucket_name + "." + url;
    }else{
        str = "https://" + bucket_name + "." + endpoint;
    }
    if(!ends_with(str,"/")){
        str = str + "/";
    }
    return str;
}

namespace aceconsider {
    namespace picbridge {

        void Config::load_config_file(path &config_file){
            string config_filename = config_file.string();
            this->config_file_path = config_filename;
            ondemand::parser parser;
            padded_string json = padded_string::load(config_filename);
            ondemand::document config = parser.iterate(json);
            string endpoint (config["endpoint"].get_string().value());
            string keyId (config["keyId"].get_string().value());
            string keySecret (config["keySecret"].get_string().value());
            string bucketName (config["bucketName"].get_string().value());
            this->endpoint = endpoint;
            this->key_id = keyId;
            this->key_secret = keySecret;
            this->bucket_name = bucketName;
            this->download_url = "ds";
            this->download_url = generate_download_url(endpoint,bucketName);
        }

        Config::Config() {
            auto userHome = std::getenv("HOME");
            path p1(userHome);
            p1 /= "/PIC_BRIDGE";
            if (!exists(p1))
            {
                boost::system::error_code ec;
                create_directories(p1,  ec);
                if(ec.failed()){
                    cerr<<"配置创建错误，请检查权限，错误信息 : "<<ec.message()<<endl;
                }
            }
            p1 /= "/config.json";
            if(!exists(p1)){
                cout<<"欢迎使用mPic"<<endl<<"这是您第一次打开程序，需要进行一些配置"<<endl;
                cout<<"请输入 endpoint:";
                cin>>endpoint;
                cout<<"请输入 Key Id:";
                cin>>key_id;
                cout<<"请输入 Key Secret:";
                cin>>key_secret;
                cout<<"请输入 Bucket Name:";
                cin>>bucket_name;

                format fmt(CONFIG_TPL);
                fmt %endpoint %key_id %key_secret %bucket_name;
                string config_str = fmt.str();
                ofstream ofs(p1.c_str());
                ofs<<config_str;
                ofs.close();
                this->download_url = generate_download_url(endpoint,bucket_name);
            }else{
                load_config_file(p1);
            }
        }

        const string &Config::getEndpoint() const {
            return endpoint;
        }

        const string &Config::getKeyId() const {
            return key_id;
        }

        const string &Config::getKeySecret() const {
            return key_secret;
        }

        const string &Config::getBucketName() const {
            return bucket_name;
        }

        Config::Config(const string &endpoint, const string &keyId, const string &keySecret, const string &bucketName)
                : endpoint(endpoint), key_id(keyId), key_secret(keySecret), bucket_name(bucketName) {
            this->download_url = generate_download_url(endpoint,bucketName);
        }

        const string &Config::getConfigFilePath() const {
            return config_file_path;
        }

        const string &Config::getDownloadUrl() const {
            return download_url;
        }

        Config::Config(const string &configFilePath) {
            path p1(configFilePath);
            load_config_file(p1);
        }

    } // aceconsider
} // picbridge