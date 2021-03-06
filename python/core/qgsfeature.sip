
typedef qint64 QgsFeatureId;
typedef QMap<int, QVariant> QgsAttributeMap;
typedef QVector<QVariant> QgsAttributes;

// QgsAttributes is implemented as a Python list of Python objects.
%MappedType QgsAttributes /DocType="list-of-attributes"/
{
%TypeHeaderCode
#include <qgsfeature.h>
%End

%ConvertFromTypeCode
    // Create the list.
    PyObject *l;

    if ( ( l = PyList_New(sipCpp->size() ) ) == NULL )
        return NULL;

    // Set the list elements.
    for ( int i = 0; i < sipCpp->size(); ++i )
    {
      QVariant* v = new QVariant( sipCpp->at( i ) );
      PyObject *tobj;

      if ( ( tobj = sipConvertFromNewType( v, sipType_QVariant,Py_None ) ) == NULL )
      {
        Py_DECREF( l );
        delete v;

        return NULL;
      }

      PyList_SET_ITEM( l, i, tobj );
    }

    return l;
%End

%ConvertToTypeCode
    // Check the type if that is all that is required.
    if (sipIsErr == NULL)
    {
      if (!PyList_Check(sipPy))
        return 0;

      for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
        if (!sipCanConvertToType(PyList_GET_ITEM(sipPy, i), sipType_QVariant, SIP_NOT_NONE))
          return 0;

      return 1;
    }

    QgsAttributes* qv = new QgsAttributes;

    for (SIP_SSIZE_T i = 0; i < PyList_GET_SIZE(sipPy); ++i)
    {
      int state;
      PyObject* obj = PyList_GET_ITEM(sipPy, i);
      QVariant *t;
      if ( obj == Py_None )
      {
        t = new QVariant( QVariant::Int );
      }
      else
      {
        t = reinterpret_cast<QVariant *>(sipConvertToType(obj, sipType_QVariant, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

        if (*sipIsErr)
        {
          sipReleaseType(t, sipType_QVariant, state);

          delete qv;
          return 0;
        }
      }

      qv->append(*t);

      sipReleaseType(t, sipType_QVariant, state);
    }

    *sipCppPtr = qv;

    return sipGetState(sipTransferObj);
%End
};

// key = feature id, value = changed attributes
typedef QMap<qint64, QMap<int, QVariant> > QgsChangedAttributesMap;

// key = feature id, value = changed geometry
typedef QMap<qint64, QgsGeometry> QgsGeometryMap;

// key = field index, value = field name
typedef QMap<int, QString> QgsFieldNameMap;

typedef QList<QgsFeature> QgsFeatureList;

typedef QMap<int, QgsField> QgsFieldMap;

/** \ingroup core
 * The feature class encapsulates a single feature including its id,
 * geometry and a list of field/values attributes.
 * \note QgsFeature objects are implicitly shared.
 * @author Gary E.Sherman
 */
class QgsFeature
{
%TypeHeaderCode
#include <qgsfeature.h>
#if (SIP_VERSION >= 0x040900 && SIP_VERSION < 0x040c01)
#define sipType_QVariant ((sipWrapperType *) sipTypeAsPyTypeObject (sipType_QVariant))
#endif
%End

  public:

    SIP_PYOBJECT __iter__();
%MethodCode
    QgsAttributes attributes = sipCpp->attributes();
    PyObject *attrs = sipConvertFromType( &attributes, sipType_QgsAttributes, Py_None );
    sipRes = PyObject_GetIter(attrs);
%End

    SIP_PYOBJECT __getitem__(int key);
%MethodCode
  QgsAttributes attrs = sipCpp->attributes();
  if (a0 < 0 || a0 >= attrs.count())
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( attrs.at(a0) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    SIP_PYOBJECT __getitem__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End

    void __setitem__(int key, QVariant value /GetWrapper/);
%MethodCode
  bool rv;

  if ( a1Wrapper == Py_None )
  {
    rv = sipCpp->setAttribute(a0, QVariant( QVariant::Int ) );
  }
  else
  {
    rv = sipCpp->setAttribute(a0, *a1);
  }

  if ( !rv )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    void __setitem__(const QString& key, QVariant value /GetWrapper/);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    if ( a1Wrapper == Py_None )
    {
      sipCpp->setAttribute(*a0, QVariant( QVariant::Int ) );
    }
    else
    {
      sipCpp->setAttribute(fieldIdx, *a1);
    }
  }
%End

    void __delitem__(int key);
%MethodCode
  if (a0 >= 0 && a0 < sipCpp->attributes().count())
    sipCpp->deleteAttribute(a0);
  else
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    void __delitem__(const QString& name);
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
    sipCpp->deleteAttribute(fieldIdx);
%End

    /** Constructor for QgsFeature
     * @param id feature id
     */
    QgsFeature( qint64 id = 0 );

    /** Constructor for QgsFeature
     * @param fields feature's fields
     * @param id feature id
     */
    QgsFeature( const QgsFields& fields, qint64 id = 0 );

    /** Copy constructor
     */
    QgsFeature( const QgsFeature & rhs );

    //! Destructor
    ~QgsFeature();

    /** Get the feature ID for this feature.
     * @returns feature ID
     * @see setFeatureId
     */
    QgsFeatureId id() const;

    /** Sets the feature ID for this feature.
     * @param id feature id
     * @see id
     */
    void setFeatureId( QgsFeatureId id );

    /** Sets the feature ID for this feature.
     * @param id feature id
     * @see id
     */
    void setId( QgsFeatureId id );

    /** Returns the feature's attributes.
     * @link attributes @endlink method.
     * @returns list of feature's attributes
     * @see setAttributes
     * @note added in QGIS 2.9
     */
    QgsAttributes attributes() const;

    /** Sets the feature's attributes.
     * @param attrs attribute list
     * @see setAttribute
     * @see attributes
     */
    void setAttributes( const QgsAttributes& attrs );

    /** Set an attribute's value by field index.
     * @param field the index of the field to set
     * @param attr the value of the attribute
     * @return false, if the field index does not exist
     * @note For Python: raises a KeyError exception instead of returning false
     * @see setAttributes
     */
    bool setAttribute( int field, const QVariant& attr /GetWrapper/);
%MethodCode
  bool rv;

  if ( a1Wrapper == Py_None )
  {
    rv = sipCpp->setAttribute(a0, QVariant( QVariant::Int ) );
  }
  else
  {
    rv = sipCpp->setAttribute(a0, *a1);
  }

  if ( !rv )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End

    /** Initialize this feature with the given number of fields. Discard any previously set attribute data.
     * @param fieldCount Number of fields to initialize
     */
    void initAttributes( int fieldCount );

    /** Deletes an attribute and its value.
     * @param field the index of the field
     * @see setAttribute
     * @note For Python: raises a KeyError exception if the field is not found
     */
    void deleteAttribute( int field );
%MethodCode
  if ( a0 >= 0 && a0 < sipCpp->attributes().count() )
    sipCpp->deleteAttribute(a0);
  else
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
%End
    /** Returns the validity of this feature. This is normally set by
     * the provider to indicate some problem that makes the feature
     * invalid or to indicate a null feature.
     * @see setValid
     */
    bool isValid() const;

    /** Sets the validity of the feature.
     * @param validity set to true if feature is valid
     * @see isValid
     */
    void setValid( bool validity );

    /** Returns true if the feature has an associated geometry.
     * @see geometry()
     * @note added in QGIS 3.0.
     */
    bool hasGeometry() const;

    /** Returns the geometry associated with this feature. If the feature has no geometry,
     * an empty QgsGeometry object will be returned.
     * @see hasGeometry()
     * @see setGeometry()
     */
    QgsGeometry geometry() const;

    /** Set the feature's geometry.
     * @param geometry new feature geometry
     * @see geometry()
     * @see clearGeometry()
     */
    void setGeometry( const QgsGeometry& geometry );

    /** Removes any geometry associated with the feature.
     * @see setGeometry()
     * @see hasGeometry()
     * @note added in QGIS 3.0
     */
    void clearGeometry();

    /** Assign a field map with the feature to allow attribute access by attribute name.
     *  @param fields The attribute fields which this feature holds
     *  @param initAttributes If true, attributes are initialized. Clears any data previously assigned.
     *                        C++: Defaults to false
     *                        Python: Defaults to true
     * @note added in QGIS 2.9
     * @see fields
     */
    void setFields( const QgsFields& fields, bool initAttributes = true );

    /** Returns the field map associated with the feature.
     * @see setFields
     */
    QgsFields fields() const;

    /** Insert a value into attribute. Returns false if attribute name could not be converted to index.
     *  Field map must be associated using @link setFields @endlink before this method can be used.
     *  @param name The name of the field to set
     *  @param value The value to set
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @see setFields
     */
    void setAttribute( const QString& name, const QVariant& value /GetWrapper/ );
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    if ( a1Wrapper == Py_None )
    {
      sipCpp->setAttribute(*a0, QVariant( QVariant::Int ) );
    }
    else
    {
      sipCpp->setAttribute(fieldIdx, *a1);
    }
  }
%End
    /** Removes an attribute value by field name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the field to delete
     *  @return false if attribute name could not be converted to index (C++ only)
     *  @note For Python: raises a KeyError exception instead of returning false
     *  @see setFields
     */
    bool deleteAttribute( const QString& name );
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    sipCpp->deleteAttribute( fieldIdx );
  }
%End
    /** Lookup attribute value from attribute name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param name The name of the attribute to get
     *  @return The value of the attribute (C++: Invalid variant if no such name exists )
     *  @note For Python: raises a KeyError exception if field is not found
     *  @see setFields
     */
    SIP_PYOBJECT attribute( const QString& name ) const;
%MethodCode
  int fieldIdx = sipCpp->fieldNameIndex(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    QVariant* v = new QVariant( sipCpp->attribute(fieldIdx) );
    sipRes = sipConvertFromNewType( v, sipType_QVariant, Py_None );
  }
%End
    /** Utility method to get attribute index from name. Field map must be associated using @link setFields @endlink
     *  before this method can be used.
     *  @param fieldName name of field to get attribute index of
     *  @returns -1 if field does not exist or field map is not associated.
     *  @see setFields
     */
    int fieldNameIndex( const QString& fieldName ) const;

    //! Allows direct construction of QVariants from features.
    operator QVariant() const;

}; // class QgsFeature

typedef QSet<qint64> QgsFeatureIds;
