/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QDomDocument>
#include <QFile>
//header for class being tested
#include <qgsrulebasedrenderer.h>

#include <qgsapplication.h>
#include <qgssymbol.h>
#include <qgsvectorlayer.h>

typedef QgsRuleBasedRenderer::Rule RRule;

class TestQgsRuleBasedRenderer: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      // we need memory provider, so make sure to load providers
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void test_load_xml()
    {
      QDomDocument doc;
      xml2domElement( "rulebasedrenderer_simple.xml", doc );
      QDomElement elem = doc.documentElement();

      QgsRuleBasedRenderer* r = static_cast<QgsRuleBasedRenderer*>( QgsRuleBasedRenderer::create( elem ) );
      QVERIFY( r );
      check_tree_valid( r->rootRule() );
      delete r;
    }

    void test_load_invalid_xml()
    {
      QDomDocument doc;
      xml2domElement( "rulebasedrenderer_invalid.xml", doc );
      QDomElement elem = doc.documentElement();

      QSharedPointer<QgsRuleBasedRenderer> r( static_cast<QgsRuleBasedRenderer*>( QgsRuleBasedRenderer::create( elem ) ) );
      QVERIFY( !r );
    }

    void test_willRenderFeature_symbolsForFeature()
    {
      // prepare features
      QgsVectorLayer* layer = new QgsVectorLayer( "point?field=fld:int", "x", "memory" );
      int idx = layer->fields().indexFromName( "fld" );
      QVERIFY( idx != -1 );
      QgsFeature f1;
      f1.initAttributes( 1 );
      f1.setAttribute( idx, QVariant( 2 ) );
      QgsFeature f2;
      f2.initAttributes( 1 );
      f2.setAttribute( idx, QVariant( 8 ) );
      QgsFeature f3;
      f3.initAttributes( 1 );
      f3.setAttribute( idx, QVariant( 100 ) );

      // prepare renderer
      QgsSymbol* s1 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
      QgsSymbol* s2 = QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry );
      RRule* rootRule = new RRule( nullptr );
      rootRule->appendChild( new RRule( s1, 0, 0, "fld >= 5 and fld <= 20" ) );
      rootRule->appendChild( new RRule( s2, 0, 0, "fld <= 10" ) );
      QgsRuleBasedRenderer r( rootRule );

      QVERIFY( r.capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature );

      QgsRenderContext ctx; // dummy render context
      ctx.expressionContext().setFields( layer->fields() );
      r.startRender( ctx, layer->fields() );

      // test willRenderFeature
      ctx.expressionContext().setFeature( f1 );
      QVERIFY( r.willRenderFeature( f1, ctx ) );
      ctx.expressionContext().setFeature( f2 );
      QVERIFY( r.willRenderFeature( f2, ctx ) );
      ctx.expressionContext().setFeature( f3 );
      QVERIFY( !r.willRenderFeature( f3, ctx ) );

      // test symbolsForFeature
      ctx.expressionContext().setFeature( f1 );
      QgsSymbolList lst1 = r.symbolsForFeature( f1, ctx );
      QVERIFY( lst1.count() == 1 );
      ctx.expressionContext().setFeature( f2 );
      QgsSymbolList lst2 = r.symbolsForFeature( f2, ctx );
      QVERIFY( lst2.count() == 2 );
      ctx.expressionContext().setFeature( f3 );
      QgsSymbolList lst3 = r.symbolsForFeature( f3, ctx );
      QVERIFY( lst3.isEmpty() );

      r.stopRender( ctx );

      delete layer;
    }

    void test_clone_ruleKey()
    {
      RRule* rootRule = new RRule( 0 );
      RRule* sub1Rule = new RRule( 0, 0, 0, "fld > 1" );
      RRule* sub2Rule = new RRule( 0, 0, 0, "fld > 2" );
      RRule* sub3Rule = new RRule( 0, 0, 0, "fld > 3" );
      rootRule->appendChild( sub1Rule );
      sub1Rule->appendChild( sub2Rule );
      sub2Rule->appendChild( sub3Rule );
      QgsRuleBasedRenderer r( rootRule );

      QgsRuleBasedRenderer* clone = static_cast<QgsRuleBasedRenderer*>( r.clone() );
      RRule* cloneRootRule = clone->rootRule();
      RRule* cloneSub1Rule = cloneRootRule->children()[0];
      RRule* cloneSub2Rule = cloneSub1Rule->children()[0];
      RRule* cloneSub3Rule = cloneSub2Rule->children()[0];

      QCOMPARE( rootRule->ruleKey(), cloneRootRule->ruleKey() );
      QCOMPARE( sub1Rule->ruleKey(), cloneSub1Rule->ruleKey() );
      QCOMPARE( sub2Rule->ruleKey(), cloneSub2Rule->ruleKey() );
      QCOMPARE( sub3Rule->ruleKey(), cloneSub3Rule->ruleKey() );

      delete clone;
    }

  private:
    void xml2domElement( const QString& testFile, QDomDocument& doc )
    {
      QString fileName = QString( TEST_DATA_DIR ) + '/' + testFile;
      QFile f( fileName );
      bool fileOpen = f.open( QIODevice::ReadOnly );
      QVERIFY( fileOpen );

      QString msg;
      int line, col;
      bool parse = doc.setContent( &f, &msg, &line, &col );
      QVERIFY( parse );
    }

    void check_tree_valid( QgsRuleBasedRenderer::Rule* root )
    {
      // root must always exist (although it does not have children)
      QVERIFY( root );
      // and does not have a parent
      QVERIFY( !root->parent() );

      Q_FOREACH ( QgsRuleBasedRenderer::Rule* node, root->children() )
        check_non_root_rule( node );
    }

    void check_non_root_rule( QgsRuleBasedRenderer::Rule* node )
    {
      qDebug() << node->dump();
      // children must not be nullptr
      QVERIFY( node );
      // and must have a parent
      QVERIFY( node->parent() );
      // check that all children are okay
      Q_FOREACH ( QgsRuleBasedRenderer::Rule* child, node->children() )
        check_non_root_rule( child );
    }

};

QTEST_MAIN( TestQgsRuleBasedRenderer )

#include "testqgsrulebasedrenderer.moc"
