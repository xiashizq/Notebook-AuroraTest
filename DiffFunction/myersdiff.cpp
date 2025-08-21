#include "myersdiff.h"
#include <QMap>
#pragma execution_character_set("utf-8")
QList<DiffLine> myersDiff(const QStringList& a, const QStringList& b) {
    int N = a.size();
    int M = b.size();

    // DP table for LCS
    std::vector<std::vector<int>> dp(N + 1, std::vector<int>(M + 1, 0));
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j)
            if (a[i] == b[j])
                dp[i + 1][j + 1] = dp[i][j] + 1;
            else
                dp[i + 1][j + 1] = std::max(dp[i][j + 1], dp[i + 1][j]);

    // Backtrack to get diff path
    int i = N, j = M;
    QList<DiffLine> result;
    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            result.prepend({a[i - 1], b[j - 1], Same});
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            result.prepend({a[i - 1], "", Removed});
            --i;
        } else {
            result.prepend({"", b[j - 1], Added});
            --j;
        }
    }
    while (i > 0) {
        result.prepend({a[i - 1], "", Removed});
        --i;
    }
    while (j > 0) {
        result.prepend({"", b[j - 1], Added});
        --j;
    }
    return result;
}
