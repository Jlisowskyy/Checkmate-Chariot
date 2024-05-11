#include <gtest/gtest.h>

#include "../include/ThreadManagement/SearchThreadManager.h"
#include "../include/Interface/FenTranslator.h"

TEST(GoCommandTest, stopCommandResponse) {
    SearchThreadManager threadManager{};
    Board board = FenTranslator::GetDefault();

    const auto Case = [&]<typename CaseFuncT> (CaseFuncT func) {
        auto t1 = std::chrono::steady_clock::now();

        func();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        threadManager.Stop();

        while (threadManager.IsSearchOn()) {
            std::this_thread::sleep_for(std::chrono::microseconds (1));
            auto t2 = std::chrono::steady_clock::now();

            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            ASSERT_LT(time_span.count(), 2.0);
        }

        auto t2 = std::chrono::steady_clock::now();
        ASSERT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count(), 1010);
    };

    auto infiniteSearch = [&]() {
        threadManager.GoInfinite(board, 0);
    };

    auto depthSearch = [&]() {
        GoInfo info{};
        info.depth = 100;

        threadManager.Go(board, 0, info);
    };

    auto moveTimeSearch = [&]() {
        GoInfo info{};
        info.timeInfo.moveTime = 1000;

        threadManager.Go(board, 0, info);
    };

    Case(infiniteSearch);
    Case(depthSearch);
    Case(moveTimeSearch);
}
