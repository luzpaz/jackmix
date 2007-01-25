/*
    Copyright ( C ) 2004 Arnold Krille <arnold@arnoldarts.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "stereo_elements.h"
#include "stereo_elements.moc"

#include "qfloatpoti.h"
#include "qfloatslider.h"
#include "slider.h"
#include "jack_backend.h"

#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QList>
#include <QtGui/QLabel>
#include <QtCore/QVariant>
#include <QtGui/QAction>

using namespace JackMix;
using namespace JackMix::MixerElements;
using namespace JackMix::MixingMatrix;

/// The Factory for creating this things...
class StereoFactory : public JackMix::MixingMatrix::ElementFactory
{
public:
	StereoFactory() : ElementFactory() { globaldebug(); }
	~StereoFactory() {}

	QStringList canCreate() const {
		return QStringList()<<"Mono2StereoElement"<<"Stereo2StereoElement";
	}
	QStringList canCreate( int in, int out ) const {
		if ( in==1 && out==2 ) return QStringList()<<"Mono2StereoElement";
		if ( in==2 && out==2 ) return QStringList()<<"Stereo2StereoElement";
		return QStringList();
	}

	Element* create( QString type , QStringList ins, QStringList outs, Widget* p, const char* n=0 ) {
		if ( type=="Mono2StereoElement" )
			return new Mono2StereoElement( ins, outs, p, n );
		if ( type=="Stereo2StereoElement" )
			return new Stereo2StereoElement( ins, outs, p, n );
		return 0;
	}
};

void MixerElements::init_stereo_elements() {
	new StereoFactory();
}


Mono2StereoElement::Mono2StereoElement( QStringList inchannel, QStringList outchannels, MixingMatrix::Widget* p, const char* n )
	: Element( inchannel, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _inchannel( inchannel[ 0 ] )
	, _outchannel1( outchannels[ 0 ] )
	, _outchannel2( outchannels[ 1 ] )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	qDebug( "Mono2StereoElement::Mono2StereoElement()" );
	float left = backend()->getVolume( _inchannel, _outchannel1 );
	float right = backend()->getVolume( _inchannel, _outchannel2 );
	qDebug( " volumes: %f, %f", left, right );
	if ( left>right ) {
		_volume_value = left;
		_balance_value = right-left;
	} else {
		_volume_value = right;
		_balance_value = right-left;
	}
	qDebug( " values: %f, %f", _volume_value, _balance_value );
	QGridLayout* _layout = new QGridLayout( this );

	menu()->addAction( "Select", this, SLOT( slot_simple_select() ) );
	menu()->addAction( "Replace", this, SLOT( slot_simple_replace() ) );

	_balance = new QFloatPoti( _balance_value, -1, 1, 100, QColor( 0,0,255 ), this );
	_layout->addWidget( _balance, 0,0 );
	connect( _balance, SIGNAL( valueChanged( float ) ), this, SLOT( balance( float ) ) );
	_volume = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 1, 3, this );
	_layout->addWidget( _volume, 1,0 );
	connect( _volume, SIGNAL( valueChanged( float ) ), this, SLOT( volume( float ) ) );
}
Mono2StereoElement::~Mono2StereoElement() {
	qDebug( "Mono2StereoElement::~Mono2StereoElement()" );
	qDebug( " volumes: %f, %f", backend()->getVolume( _inchannel, _outchannel1 ), backend()->getVolume( _inchannel, _outchannel2 ) );
	qDebug( " values: %f, %f", _volume_value, _balance_value );
}


void Mono2StereoElement::balance( float n ) {
	//qDebug( "Mono2StereoElement::balance( float %f )", n );
	_balance_value = n;
	calculateVolumes();
	_balance->setValue( n );
	emit valueChanged( this, QString( "balance" ) );
}
void Mono2StereoElement::volume( float n ) {
	//qDebug( "Mono2StereoElement::volume( float %f )", n );
	_volume_value = n;
	calculateVolumes();
	_volume->value( n );
	emit valueChanged( this, QString( "volume" ) );
}

void Mono2StereoElement::calculateVolumes() {
	float left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );
	backend()->setVolume( _inchannel, _outchannel1, left );
	backend()->setVolume( _inchannel, _outchannel2, right );
}

void Mono2StereoElement::slave() {
	emit connectSlave( this, QString( "volume" ) );
}
void Mono2StereoElement::deslave() {
	emit disconnectSlave( this, QString( "volume" ) );
}



Stereo2StereoElement::Stereo2StereoElement( QStringList inchannels, QStringList outchannels, MixingMatrix::Widget* p, const char* n )
	: Element( inchannels, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _inchannel1( inchannels[ 0 ] )
	, _inchannel2( inchannels[ 1 ] )
	, _outchannel1( outchannels[ 0 ] )
	, _outchannel2( outchannels[ 1 ] )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	backend()->setVolume( _inchannel1, _outchannel2, 0 );
	backend()->setVolume( _inchannel2, _outchannel1, 0 );
	float left = backend()->getVolume( _inchannel1, _outchannel1 );
	float right = backend()->getVolume( _inchannel2, _outchannel2 );
	if ( left>right ) {
		_volume_value = left;
		_balance_value = left-right;
	} else {
		_volume_value = right;
		_balance_value = left-right;
	}
	QGridLayout* _layout = new QGridLayout( this );
	_balance_widget = new JackMix::GUI::Slider( _balance_value, -1, 1, 2, 0.1, this, "%1" );
	_layout->addWidget( _balance_widget, 0,0, 1,1 );
	_layout->setRowStretch( 0, 0 );
	connect( _balance_widget, SIGNAL( valueChanged( float ) ), this, SLOT( balance( float ) ) );
	_volume_widget = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 1, 3, this );
	_layout->addWidget( _volume_widget, 1,1, 1,1 );
	_layout->setRowStretch( 1, 1000 );
	connect( _volume_widget, SIGNAL( valueChanged( float ) ), this, SLOT( volume( float ) ) );

	QAction *toggle = new QAction( "Toggle Selection", this );
	connect( toggle, SIGNAL( activated() ), this, SLOT( slot_simple_select() ) );
	menu()->addAction( toggle );
	QAction *replace = new QAction( "Replace", this );
	connect( replace, SIGNAL( activated() ), this, SLOT( slot_simple_replace() ) );
	menu()->addAction( replace );
	menu()->addSeparator();
	QAction *disconnectM = new QAction( "Disconnect Master: volume", this );
	connect( disconnectM, SIGNAL( activated() ), this, SLOT( disconnectM() ) );
	menu()->addAction( disconnectM );
}
Stereo2StereoElement::~Stereo2StereoElement() {
}

void Stereo2StereoElement::disconnectM() { emit disconnectMaster( this, "volume" ); }

void Stereo2StereoElement::balance( float n ) {
	//qDebug( "Mono2StereoElement::balance( float %f )", n );
	_balance_value = n;
	_balance_widget->value( n );
	calculateVolumes();
	emit valueChanged( this, QString( "balance" ) );
}
void Stereo2StereoElement::volume( float n ) {
	//qDebug( "Mono2StereoElement::volume( float %f )", n );
	_volume_value = n;
	_volume_widget->value( n );
	calculateVolumes();
	emit valueChanged( this, QString( "volume" ) );
}

void Stereo2StereoElement::calculateVolumes() {
	float left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );
	backend()->setVolume( _inchannel1, _outchannel1, left );
	backend()->setVolume( _inchannel2, _outchannel2, right );
}
