#include "CCLOPSyntaxHighlighter.h"

CCLOPSyntaxHighlighter::CCLOPSyntaxHighlighter(QTextEdit *parent):
 QSyntaxHighlighter(parent)
{
     QStringList keywordPatterns;
     keywordPatterns <<
      "^Name" <<
      "^Script" <<
      "^LinearParameter" <<
      "^Processor" <<
      "^IntegerParameter" <<
      "^GammaParameter" <<
      "^IntegerGammaParameter" <<
      "^Replications" <<
      "^DrawElo" <<
      "^StopOnError" <<
      "^H" <<
      "^Correlations";

     foreach (const QString &pattern, keywordPatterns)
     {
         HighlightingRule rule;
         rule.pattern = QRegExp(pattern);
         rule.format.setForeground(Qt::darkMagenta);
         highlightingRules.append(rule);
     }

     {
         HighlightingRule rule;
         rule.pattern = QRegExp("^#[^\n]*");
         rule.format.setForeground(Qt::darkGreen);
         highlightingRules.append(rule);
     }
}

void CCLOPSyntaxHighlighter::highlightBlock(const QString &text)
{
     foreach (const HighlightingRule &rule, highlightingRules) {
         QRegExp expression(rule.pattern);
         int index = expression.indexIn(text);
         while (index >= 0) {
             int length = expression.matchedLength();
             setFormat(index, length, rule.format);
             index = expression.indexIn(text, index + length);
         }
     }
}
