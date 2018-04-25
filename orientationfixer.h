#ifndef FIXORIENTATIONS_H
#define FIXORIENTATIONS_H

#include "extraction_config.h"

class OrientationFixer : public QObject
{
    Q_OBJECT

public:
    OrientationFixer();

    void setParams(const PREPROCESSING_RESULTS &input, const QVector<MINUTIA> &minutiae, const QVector<MINUTIA> &invertedMinutiae);
    void finalizeDirections(QVector<MINUTIA> &minutiae);
    void fix();

    //getNset
    QVector<MINUTIA> getFixedMinutiae() const;

private:

    PREPROCESSING_RESULTS input;
    QVector<MINUTIA> minutiae;
    QVector<MINUTIA> invertedMinutiae;

    QVector<MINUTIA> fixedMinutiae;

    void fixEndings();
    void fixBifurcations();

signals:
    void extractionErrorSignal(int errorCode);

};

#endif // FIXORIENTATIONS_H
