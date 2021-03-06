/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GEOMAP_INCLUDED
#define GEOMAP_INCLUDED

#include "../grinliz/gltypes.h"
#include "../grinliz/gldebug.h"
#include "../engine/surface.h"

#include "../engine/map.h"

class SpaceTree;
class Texture;
class Model;

// Note this is the lightweight iMap, NOT Map
class GeoMap :	public IMap,
				public ITextureCreator
{
public:
	GeoMap( SpaceTree* tree );
	~GeoMap();

	// IMap
	virtual Texture* LightFogMapTexture()		{ return dayNightTex; }
	virtual void LightFogMapParam( float* w, float* h, float* dx, float* dy );

	// ITextureCreator
	virtual void CreateTexture( Texture* t );

	// GeoMap functionality
	void DoTick( U32 currentTime, U32 deltaTime );

	bool GetDayTime( float x );

	enum {
		MAP_X = 20,
		MAP_Y = 10
	};

private:
	enum {
		DAYNIGHT_TEX_SIZE = 32
	};

	float				scrolling;
	float				dayNightOffset;

	SpaceTree*			tree;
	Texture*			dayNightTex;
	Surface				dayNightSurface;
	Model*				geoModel[2];
};

#endif // GEOMAP_INCLUDED