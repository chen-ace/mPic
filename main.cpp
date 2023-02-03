#include <iostream>
#include <alibabacloud/oss/OssClient.h>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <fstream>

#include "Config.h"

using namespace std;
using namespace AlibabaCloud::OSS;
using namespace boost;
using namespace boost::program_options;
using namespace boost::filesystem;
using namespace boost::uuids;
using namespace aceconsider::picbridge;

unique_ptr<Config> config;
bool rename_flag = false;
vector<string> upload_files;

string CONFIG_TPL = R"(
{
    "endpoint": "%s",
    "keyId": "%s",
    "keySecret": "%s",
    "bucketName":"%s"
}
)";

void init(){
    auto userHome = std::getenv("HOME");
    path p1(userHome);
    p1 /= "/mPic";
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
        string endpoint,key_id,key_secret,bucket_name;
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
        cout<<"配置文件写入成功！ 配置文件路径为： "<< p1<<endl;
    }
}

void parse_cmd(int argc,char ** argv){
    options_description opts("mPic可用选项");
    opts.add_options()
            ("help,h","帮助信息")
            ("endpoint,e",value<string>(),"Endpoint")
            ("id,i",value<string>(),"Access Key Id")
            ("secret,s",value<string>(),"Access Key Secret")
            ("bucket,b",value<string>(),"Bucket Name")
            ("config,c",value<string>(),"外部配置文件\n默认使用当前用户目录下的配置文件")
            ("rename,r",value<bool>()->zero_tokens(),"将文件进行重命名\n默认否")
            ("files,f",value<vector<string>>(&upload_files)->multitoken(),"需要上传的文件列表")
            ;

    variables_map vm;
    store(parse_command_line(argc,argv,opts),vm);
    notify(vm);

    if(vm.empty())
    {
        cout<<opts<<endl;
        std::exit(EXIT_SUCCESS);
    }
    if(vm.count("help"))
    {
        cout<<opts<<endl;
        std::exit(EXIT_SUCCESS);
    }
    if(!vm.count("files")){
        cout<<opts<<endl;
        std::exit(EXIT_SUCCESS);
    }
    if(upload_files.empty()){
        cout<<opts<<endl;
        std::exit(EXIT_SUCCESS);
    }

    if(vm.count("rename")){
        rename_flag = true;
    }
    if(vm.count("config")){
        Config config2(vm["config"].as<string>());
        config = make_unique<Config>(Config(vm["config"].as<string>()));
    }else if(vm.count("endpoint") && vm.count("id")
             && vm.count("secret") && vm.count("bucket")){
        auto endpoint = vm["endpoint"].as<string>();
        auto id = vm["id"].as<string>();
        auto secret = vm["secret"].as<string>();
        auto bucket = vm["bucket"].as<string>();
        config = make_unique<Config>(Config(endpoint,id,secret,bucket));
    }else{
        config = make_unique<Config>(Config());
    }
}


int main(int argc,char ** argv) {
    init();
    parse_cmd(argc,argv);
    // Initialize the SDK
    InitializeSdk();

    // Configure the instance
    ClientConfiguration conf;
    OssClient client(config->getEndpoint(),
                     config->getKeyId(),
                     config->getKeySecret(),
                     conf);

    vector<string> result;
    for (const auto &item: upload_files){
        path p(item);
        string filename;
        if(rename_flag){
            random_generator uuid_gen;
            boost::uuids::uuid fu = uuid_gen();
            filename = to_string(fu)+p.extension().string();
        }else{
            filename = p.filename().string();
        }

        // 上传时禁止覆盖同名文件
        ObjectMetaData meta;
        meta.addHeader("x-oss-forbid-overwrite", "true");
        auto outcome =
                client.PutObject(config->getBucketName(),filename,item,meta);
        if (!outcome.isSuccess()) {
            std::cerr << "fail" <<
                        "origin_file:"<<item<<",upload_file:"<<filename<<
                      ",code:" << outcome.error().Code() <<
                      ",message:" << outcome.error().Message() <<
                      ",requestId:" << outcome.error().RequestId() << std::endl;
            result.push_back("error");
        }else{
            result.push_back(config->getDownloadUrl()+filename);
        }
    }
    // Close the SDK
    ShutdownSdk();

    cout<<"Upload Success:"<<endl;
    for (const auto &item: result){
        cout<<item<<endl;
    }
    return 0;
}
