//
// Created by Addison on 2023/2/3.
//
#ifndef PICBRIDGE_CONFIG_H
#define PICBRIDGE_CONFIG_H

#include <boost/filesystem.hpp>

namespace aceconsider {
    namespace picbridge {

        /**
         * 提供配置信息
         */
        class Config {
        public:
            /**
             * 从~/PIC_BRIDGE/config.json中载入配置
             */
            Config();

            /**
             * 提供配置信息，不载入配置文件
             * @param endpoint
             * @param keyId
             * @param keySecret
             * @param bucketName
             */
            Config(const std::string &endpoint, const std::string &keyId, const std::string &keySecret,
                   const std::string &bucketName);

            /**
             * 载入提供的配置文件
             * @param configFilePath 配置文件路径
             */
            explicit Config(const std::string &configFilePath);

        private:
            std::string config_file_path;
            std::string endpoint;
            std::string key_id;
            std::string key_secret;
            std::string bucket_name;
            std::string download_url;
        public:
            const std::string &getConfigFilePath() const;

            const std::string &getEndpoint() const;

            const std::string &getKeyId() const;

            const std::string &getKeySecret() const;

            const std::string &getBucketName() const;

            const std::string &getDownloadUrl() const;

        private:
            void load_config_file(boost::filesystem::path &config_file);
        };


    } // aceconsider
} // picbridge

#endif //PICBRIDGE_CONFIG_H
