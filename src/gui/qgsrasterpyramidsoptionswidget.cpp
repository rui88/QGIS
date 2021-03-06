/***************************************************************************
                          qgsrasterpyramidsoptionswidget.cpp
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterpyramidsoptionswidget.h"
#include "qgsrasterdataprovider.h"
#include "qgslogger.h"
#include "qgsdialog.h"

#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QCheckBox>

QgsRasterPyramidsOptionsWidget::QgsRasterPyramidsOptionsWidget( QWidget* parent, const QString& provider )
    : QWidget( parent )
    , mProvider( provider )
{
  setupUi( this );

  mSaveOptionsWidget->setProvider( provider );
  mSaveOptionsWidget->setPyramidsFormat( QgsRaster::PyramidsGTiff );
  mSaveOptionsWidget->setType( QgsRasterFormatSaveOptionsWidget::ProfileLineEdit );

  updateUi();
}

QgsRasterPyramidsOptionsWidget::~QgsRasterPyramidsOptionsWidget()
{
}


void QgsRasterPyramidsOptionsWidget::updateUi()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // keep it in sync with qgsrasterlayerproperties.cpp
  tmpStr = mySettings.value( prefix + "format", "external" ).toString();
  if ( tmpStr == "internal" )
    cbxPyramidsFormat->setCurrentIndex( INTERNAL );
  else if ( tmpStr == "external_erdas" )
    cbxPyramidsFormat->setCurrentIndex( ERDAS );
  else
    cbxPyramidsFormat->setCurrentIndex( GTIFF );

  // initialize resampling methods
  cboResamplingMethod->clear();
  QPair<QString, QString> method;
  Q_FOREACH ( method, QgsRasterDataProvider::pyramidResamplingMethods( mProvider ) )
  {
    cboResamplingMethod->addItem( method.second, method.first );
  }
  QString defaultMethod = mySettings.value( prefix + "resampling", "AVERAGE" ).toString();
  int idx = cboResamplingMethod->findData( defaultMethod );
  cboResamplingMethod->setCurrentIndex( idx );

  // validate string, only space-separated positive integers are allowed
  lePyramidsLevels->setEnabled( cbxPyramidsLevelsCustom->isChecked() );
  lePyramidsLevels->setValidator( new QRegExpValidator( QRegExp( "(\\d*)(\\s\\d*)*" ), lePyramidsLevels ) );
  connect( lePyramidsLevels, SIGNAL( textEdited( const QString & ) ),
           this, SLOT( setOverviewList() ) );

  // overview list
  if ( mOverviewCheckBoxes.isEmpty() )
  {
    QList<int> overviewList;
    overviewList << 2 << 4 << 8 << 16 << 32 << 64;
    mOverviewCheckBoxes.clear();
    Q_FOREACH ( int i, overviewList )
    {
      mOverviewCheckBoxes[ i ] = new QCheckBox( QString::number( i ), this );
      connect( mOverviewCheckBoxes[ i ], SIGNAL( toggled( bool ) ),
               this, SLOT( setOverviewList() ) );
      layoutPyramidsLevels->addWidget( mOverviewCheckBoxes[ i ] );
    }
  }
  else
  {
    Q_FOREACH ( int i, mOverviewCheckBoxes.keys() )
      mOverviewCheckBoxes[ i ]->setChecked( false );
  }
  tmpStr = mySettings.value( prefix + "overviewList", "" ).toString();
  Q_FOREACH ( const QString& lev, tmpStr.split( ' ', QString::SkipEmptyParts ) )
  {
    if ( mOverviewCheckBoxes.contains( lev.toInt() ) )
      mOverviewCheckBoxes[ lev.toInt()]->setChecked( true );
  }
  setOverviewList();

  mSaveOptionsWidget->updateProfiles();

  connect( cbxPyramidsFormat, SIGNAL( currentIndexChanged( int ) ),
           this, SIGNAL( someValueChanged() ) );
  connect( cboResamplingMethod, SIGNAL( currentIndexChanged( int ) ),
           this, SIGNAL( someValueChanged() ) );
  connect( mSaveOptionsWidget, SIGNAL( optionsChanged() ),
           this, SIGNAL( someValueChanged() ) );
}

QString QgsRasterPyramidsOptionsWidget::resamplingMethod() const
{
  return cboResamplingMethod->currentData().toString();
}

void QgsRasterPyramidsOptionsWidget::apply()
{
  QSettings mySettings;
  QString prefix = mProvider + "/driverOptions/_pyramids/";
  QString tmpStr;

  // mySettings.setValue( prefix + "internal", cbxPyramidsInternal->isChecked() );
  if ( cbxPyramidsFormat->currentIndex() == INTERNAL )
    tmpStr = "internal";
  else if ( cbxPyramidsFormat->currentIndex() == ERDAS )
    tmpStr = "external_erdas";
  else
    tmpStr = "external";
  mySettings.setValue( prefix + "format", tmpStr );
  mySettings.setValue( prefix + "resampling", resamplingMethod() );
  mySettings.setValue( prefix + "overviewStr", lePyramidsLevels->text().trimmed() );

  // overview list
  tmpStr = "";
  Q_FOREACH ( int i, mOverviewCheckBoxes.keys() )
  {
    if ( mOverviewCheckBoxes[ i ]->isChecked() )
      tmpStr += QString::number( i ) + ' ';
  }
  mySettings.setValue( prefix + "overviewList", tmpStr.trimmed() );

  mSaveOptionsWidget->apply();
}

void QgsRasterPyramidsOptionsWidget::checkAllLevels( bool checked )
{
  Q_FOREACH ( int i, mOverviewCheckBoxes.keys() )
    mOverviewCheckBoxes[ i ]->setChecked( checked );
}

void QgsRasterPyramidsOptionsWidget::on_cbxPyramidsLevelsCustom_toggled( bool toggled )
{
  // if toggled, disable checkboxes and enable line edit
  lePyramidsLevels->setEnabled( toggled );
  Q_FOREACH ( int i, mOverviewCheckBoxes.keys() )
    mOverviewCheckBoxes[ i ]->setEnabled( ! toggled );
  setOverviewList();
}

void QgsRasterPyramidsOptionsWidget::on_cbxPyramidsFormat_currentIndexChanged( int index )
{
  mSaveOptionsWidget->setEnabled( index != ERDAS );
  QgsRaster::RasterPyramidsFormat format;
  switch ( index )
  {
    case GTIFF:
      format = QgsRaster::PyramidsGTiff;
      break;
    case INTERNAL:
      format = QgsRaster::PyramidsInternal;
      break;
    case ERDAS:
      format = QgsRaster::PyramidsErdas;
      break;
    default:
      QgsDebugMsg( "Should not happen !" );
      format = QgsRaster::PyramidsGTiff;
      break;
  }
  mSaveOptionsWidget->setPyramidsFormat( format );
}

void QgsRasterPyramidsOptionsWidget::setOverviewList()
{

  mOverviewList.clear();

  // if custom levels is toggled, get selection from line edit
  if ( cbxPyramidsLevelsCustom->isChecked() )
  {
    // should we also validate that numbers are increasing?
    Q_FOREACH ( const QString& lev, lePyramidsLevels->text().trimmed().split( ' ', QString::SkipEmptyParts ) )
    {
      QgsDebugMsg( "lev= " + lev );
      int tmpInt = lev.toInt();
      if ( tmpInt > 0 )
      {
        QgsDebugMsg( "tmpInt= " + QString::number( tmpInt ) );
        // if number is valid, add to overview list
        mOverviewList << tmpInt;
      }
    }
  }
  else
  {
    Q_FOREACH ( int i, mOverviewCheckBoxes.keys() )
    {
      if ( mOverviewCheckBoxes[ i ]->isChecked() )
        mOverviewList << i;
    }
  }

  emit overviewListChanged();
}
