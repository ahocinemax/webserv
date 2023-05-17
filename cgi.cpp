#include "includes/CgiHandler.hpp"
#include <iostream>

int main() {
    CgiHandler cgiHandler;

    // CGIスクリプトのパスを設定
    //cgiHandler.setScriptPath("/path/to/your/tohoho.pl");

    // CGIプログラムのパスを設定 (perlの場合)
    //cgiHandler.setProgram("/usr/local/bin/perl");

    // CGIスクリプトの実行結果を格納するための変数
    std::string output;

    // CGIスクリプトの実行
    if (cgiHandler.getCgiOutput(output)) {
        // 実行結果の出力
        std::cout << output << std::endl;
        std::cout << "program executed" <<std::endl;
    } else {
        // エラーメッセージの出力
        std::cerr << "Failed to execute CGI script." << std::endl;
    }

    return 0;
}
