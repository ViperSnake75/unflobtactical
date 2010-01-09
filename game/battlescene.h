#ifndef UFOATTACK_BATTLE_SCENE_INCLUDED
#define UFOATTACK_BATTLE_SCENE_INCLUDED

#include "scene.h"
#include "unit.h"
#include "../engine/ufoutil.h"
#include "gamelimits.h"
#include "../grinliz/glbitarray.h"
#include "../grinliz/glvector.h"
#include "../sqlite3/sqlite3.h"

class Model;
class UIButtonBox;
class UIButtonGroup;
class Engine;
class Texture;


class BattleScene : public Scene
{
public:
	BattleScene( Game* );
	virtual ~BattleScene();

	virtual void Tap(	int count, 
						const grinliz::Vector2I& screen,
						const grinliz::Ray& world );

	virtual void Drag(	int action,
						const grinliz::Vector2I& view );

	virtual void Zoom( int action, int distance );
	virtual void Rotate( int aciton, float degreesFromStart );
	virtual void CancelInput() {}

	virtual void DoTick( U32 currentTime, U32 deltaTime );
	virtual void DrawHUD();

	virtual void Save( UFOStream* s );
	virtual void Load( UFOStream* s );

	// debugging / MapMaker
	void MouseMove( int x, int y );
#ifdef MAPMAKER
	void RotateSelection( int delta );
	void DeleteAtSelection();
	void DeltaCurrentMapItem( int d );
#endif

private:
	enum {
		BTN_TAKE_OFF,
		BTN_END_TURN,
		BTN_NEXT,
		BTN_NEXT_DONE,
		BTN_TARGET,
		BTN_LEFT,
		BTN_RIGHT,
		BTN_CHAR_SCREEN
	};

	enum {
		ACTION_NONE,
		ACTION_MOVE,
		ACTION_ROTATE,
		ACTION_SHOOT,
		ACTION_DELAY,
		ACTION_HIT,
		ACTION_CAMERA,
	};

	struct MoveAction	{
		int pathStep;
		float pathFraction;
	};

	struct RotateAction {
		float rotation;
	};

	struct ShootAction {
		grinliz::Vector3F target;
		int select;	// primary{0) or secondary(1)
	};

	struct DelayAction {
		U32 delay;
	};

	struct HitAction {
		DamageDesc			damageDesc;		// hit with what??
		bool				explosive;
		grinliz::Vector3F	p;				// point of impact
		grinliz::Vector3F	n;				// normal from shooter to target
		Model*				m;				// model impacted - may be 0
	};

	struct CameraAction {
		grinliz::Vector3F	start;
		grinliz::Vector3F	end;
		int					time;
		int					timeLeft;
	};

	struct Action
	{
		int action;
		Unit* unit;			// unit performing the action (sometimes null)

		union {
			MoveAction		move;
			RotateAction	rotate;
			ShootAction		shoot;
			DelayAction		delay;
			HitAction		hit;
			CameraAction	camera;
		} type;

		void Clear()							{ action = ACTION_NONE; memset( &type, 0, sizeof( type ) ); }
		void Init( int id, Unit* unit )			{ Clear(); action = id; this->unit = unit; }
		bool NoAction()							{ return action == ACTION_NONE; }
	};

	void InitAction( Action* a, int action ) {
		memset( a, 0, sizeof(Action) );
		a->action = action;
	}

	CStack< Action > actionStack;

	void PushRotateAction( Unit* src, const grinliz::Vector3F& dst, bool quantize );
	
	// Try to shoot. Return true if success.
	bool PushShootAction(	Unit* src, const grinliz::Vector3F& dst, 
							int select, int type, bool useError );

	bool ProcessAction( U32 deltaTime );
	bool ProcessActionShoot( Action* action, Unit* unit, Model* model );
	bool ProcessActionHit( Action* action );	
	void ScrollOnScreen( const grinliz::Vector3F& v );

	void StopForNewTeamTarget();
	void DoReactionFire();

	struct Path
	{
		grinliz::Vector2<S16>	start, end;
		std::vector< void* >	statePath;

		grinliz::Vector2<S16> GetPathAt( unsigned i ) {
			grinliz::Vector2<S16> v = *((grinliz::Vector2<S16>*)&statePath[i] );
			return v;
		}
		void Clear() { start.Set( -1, -1 ); end.Set( -1, -1 ); }
		void CalcDelta( int i0, int i1, grinliz::Vector2I* vec, float* rot );
		void Travel( float* travelDistance, int* pathPos, float* fraction );
		void GetPos( int step, float fraction, float* x, float* z, float* rot );
	private:
		float DeltaToRotation( int dx, int dy );
	};
	Path path;

	// Show the UI zones arount the selected unit
	void ShowNearPath( const Unit* unit );
	// set the fire widget to the primary and secondary weapon
	void SetFireWidget();
	float Travel( U32 timeMSec, float speed ) { return speed * (float)timeMSec / 1000.0f; }

	void InitUnits();
	void TestHitTesting();
	void SetPathBlocks();

	Unit* UnitFromModel( Model* m );
	Unit* GetUnitFromTile( int x, int z );
	bool HandleIconTap( int screenX, int screenY );
	void SetFogOfWar();

	struct Selection
	{
		Selection()	{ Clear(); }
		void Clear()		{ soldierUnit = 0; targetUnit = 0; targetPos.Set( -1, -1 ); }
		void ClearTarget()	{ targetUnit = 0; targetPos.Set( -1, -1 ); }
		Unit*	soldierUnit;
		
		Unit*				targetUnit;
		grinliz::Vector2I	targetPos;
	};
	Selection selection;

	bool	SelectedSoldier()		{ return selection.soldierUnit != 0; }
	Unit*	SelectedSoldierUnit()	{ return selection.soldierUnit; }
	Model*	SelectedSoldierModel()	{ if ( selection.soldierUnit ) return selection.soldierUnit->GetModel(); return 0; }

	bool	HasTarget()				{ return selection.targetUnit || selection.targetPos.x >= 0; }
	bool	AlienTargeted()			{ return selection.targetUnit != 0; }
	Unit*	AlienUnit()				{ return selection.targetUnit; }
	Model*	AlienModel()			{ if ( selection.targetUnit ) return selection.targetUnit->GetModel(); return 0; }

	void	SetSelection( Unit* unit );

	void NewTurn( int team );

	grinliz::Vector3F dragStart;
	grinliz::Vector3F dragStartCameraWC;
	grinliz::Matrix4  dragMVPI;

	int		initZoomDistance;
	float	initZoom;
	
	grinliz::Vector3F orbitPole;
	float	orbitStart;

	enum {
		UIM_NORMAL,			// normal click and move
		UIM_TARGET_TILE,	// special target abitrary tile mode
		UIM_FIRE_MENU		// fire menu is up
	};

	int				uiMode;
	UIButtonGroup*	widgets;
	UIButtonBox*	fireWidget;
	Engine*			engine;
	grinliz::Random random;	// "the" random number generator for the battle
	int				currentTeamTurn;

	enum {
		MAX_TERRANS = 8,
		MAX_CIVS = 16,
		MAX_ALIENS = 16,

		TERRAN_UNITS_START	= 0,
		TERRAN_UNITS_END	= 8,
		CIV_UNITS_START		= TERRAN_UNITS_END,
		CIV_UNITS_END		= CIV_UNITS_START+MAX_CIVS,
		ALIEN_UNITS_START	= CIV_UNITS_END,
		ALIEN_UNITS_END		= ALIEN_UNITS_START+MAX_ALIENS,
		// const int MAX_UNITS	= 40
	};


	struct TargetEvent
	{
		U8 team;		// 1: team, 0: unit
		U8 gain;		// 1: gain, 0: loss
		U8 viewerID;	// unit id of viewer, or teamID if team event
		U8 targetID;	// unit id of target
	};

	// Note that this structure gets copied POD style.
	//
	// Terran is enemy of Alien and vice versa. Civs aren't
	// counted, which means they have to be queried and 
	// don't impact reaction fire.
	class Targets
	{
	public:
		Targets() { Clear(); }

		void Clear() {
			targets.ClearAll();
			teamTargets.ClearAll();
			memset( teamCount, 0, 9*sizeof(int) );
		}
		static int Team( int id ) {
			if ( id >= TERRAN_UNITS_START && id < TERRAN_UNITS_END )
				return Unit::SOLDIER;
			else if ( id >= CIV_UNITS_START && id < CIV_UNITS_END ) 
				return Unit::CIVILIAN;
			else if ( id >= ALIEN_UNITS_START && id < ALIEN_UNITS_END ) 
				return Unit::ALIEN;
			else { 
				GLASSERT( 0 ); 
				return 0;
			}
		}
		void Set( int viewer, int target )		{ 
			targets.Set( viewer, target, 0 );
			if ( !teamTargets.IsSet( Team( viewer ), target ) ) 
				teamCount[ Team( viewer ) ][ Team( target ) ] += 1;
			teamTargets.Set( Team( viewer ), target );
		}
		int CanSee( int viewer, int target )	{
			return targets.IsSet( viewer, target, 0 );
		}
		int TeamCanSee( int viewerTeam, int target ) {
			return teamTargets.IsSet( viewerTeam, target, 0 );
		}
		int TotalTeamCanSee( int viewerTeam, int targetTeam ) {
			return teamCount[ viewerTeam ][ targetTeam ];
		}
		int CalcTotalUnitCanSee( int viewer, int targetTeam ) {
			int start[] = { TERRAN_UNITS_START, CIV_UNITS_START, ALIEN_UNITS_START };
			int end[]   = { TERRAN_UNITS_END, CIV_UNITS_END, ALIEN_UNITS_END };
			int count = 0;
			GLASSERT( targetTeam >= 0 && targetTeam < 3 );
			for( int i=start[targetTeam]; i<end[targetTeam]; ++i ) {
				if ( targets.IsSet( viewer, i, 0 ) )
					++count;
			}
			return count;
		}

	private:
		grinliz::BitArray< MAX_UNITS, MAX_UNITS, 1 > targets;
		grinliz::BitArray< 3, MAX_UNITS, 1 > teamTargets;
		int teamCount[3][3];
	};

	Targets m_targets;
	CDynArray< TargetEvent > targetEvents;
	CDynArray< grinliz::Vector2I > doors;
	void ProcessDoors();

	// Updates what units can and can not see. Sets the 'Targets' structure above,
	// and generates targetEvents.
	void CalcTeamTargets();
	void DumpTargetEvents();

	void InvalidateAllVisibility();
	void InvalidateAllVisibility( const grinliz::Rectangle2I& bounds );
	void CalcAllVisibility();

	// before using 'visibilityMap', bring current with CalcAllVisibility()
	grinliz::BitArray< MAP_SIZE, MAP_SIZE, MAX_UNITS > visibilityMap;
	
	grinliz::BitArray< MAP_SIZE, MAP_SIZE, 1 > visibilityProcessed;		// temporary - used in vis calc.
	void CalcUnitVisibility( const Unit* unit );
	void CalcVisibilityRay(	int unitID,
							const grinliz::Vector2I& pos,
							const grinliz::Vector2I& origin, 
							const grinliz::Vector2I& facing );

	Unit units[MAX_UNITS];

#ifdef MAPMAKER
	// Mapmaker:
	void UpdatePreview();
	Model* mapSelection;
	Model* preview;
	int currentMapItem;
	sqlite3* mapDatabase;
#endif
};


#endif // UFOATTACK_BATTLE_SCENE_INCLUDED