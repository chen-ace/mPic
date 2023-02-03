#include <iostream>
#include <alibabacloud/oss/OssClient.h>
#include <boost/program_options.hpp>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "Config.h"

using namespace std;
using namespace AlibabaCloud::OSS;
using namespace boost::program_options;
using namespace boost::filesystem;
using namespace boost::uuids;
using namespace aceconsider::picbridge;

unique_ptr<Config> config;
bool rename_flag = false;
vector<string> upload_files;

void parse_cmd(int argc,char ** argv){
    options_description opts("PicBridge可用选项");
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
        return;
    }
    if(vm.count("help"))
    {
        cout<<opts<<endl;
        return;
    }
    if(!vm.count("files")){
        cout<<opts<<endl;
        return;
    }
    if(upload_files.empty()){
        cout<<opts<<endl;
        return;
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
