#include "myersdiff.h"
#include <QMap>
#pragma execution_character_set("utf-8")
QList<Diff::Line> myersDiff(const QStringList& a, const QStringList& b) {
    int N = a.size();
    int M = b.size();
    std::vector<std::vector<int>> dp(N + 1, std::vector<int>(M + 1, 0));

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < M; ++j)
            if (a[i] == b[j])
                dp[i + 1][j + 1] = dp[i][j] + 1;
            else
                dp[i + 1][j + 1] = std::max(dp[i][j + 1], dp[i + 1][j]);

    int i = N, j = M;
    QList<Diff::Line> result;

    while (i > 0 && j > 0) {
        if (a[i - 1] == b[j - 1]) {
            result.prepend(Diff::Line(a[i - 1], b[j - 1], Diff::Type::Same));
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            result.prepend(Diff::Line(a[i - 1], "", Diff::Type::Removed));
            --i;
        } else {
            result.prepend(Diff::Line("", b[j - 1], Diff::Type::Added));
            --j;
        }
    }
    while (i > 0) {
        result.prepend(Diff::Line(a[i - 1], "", Diff::Type::Removed));
        --i;
    }
    while (j > 0) {
        result.prepend(Diff::Line("", b[j - 1], Diff::Type::Added));
        --j;
    }
    return result;
}
