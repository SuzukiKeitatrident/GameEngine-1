//--------------------------------------------------------------------------------------
// ファイル名: Enemy.h
// 作成者:
// 作成日:
// 説明:
//--------------------------------------------------------------------------------------

#include "Enemy.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

/*==================================
目的の角度への最短角度を取得（ラジアン）

引数	_angle0	ベースとなる角度
_angle1	目標とする角度
戻り値	差分角度

角度０から角度１に最短コースで向かう際に加算する角度を取得する
===================================*/
static float GetShortAngleRad(float _angle0, float _angle1)
{
	float angle_sub;

	angle_sub = _angle1 - _angle0;	// 角度の差
	// 差が１８０度(π）より大きかった場合、逆回転の方が近いので、マイナスに変換
	// 最終的に-180〜+180度の範囲に。
	if (angle_sub > XM_PI)
	{
		angle_sub -= XM_2PI;
	}
	else if (angle_sub < -XM_PI)
	{
		angle_sub += XM_2PI;
	}

	return angle_sub;
}

//-----------------------------------------------------------------------------
// コンストラクタ
//-----------------------------------------------------------------------------
Enemy::Enemy()
: m_cycle(0.0f)
, m_Death(false)
{
	m_Timer = 60;
}

//-----------------------------------------------------------------------------
// デストラクタ
//-----------------------------------------------------------------------------
Enemy::~Enemy()
{
}

//-----------------------------------------------------------------------------
// 初期化
//-----------------------------------------------------------------------------
void Enemy::Initialize()
{
	m_Obj.resize(PARTS_NUM);
	m_Obj[PARTS_BODY].LoadModelFile(L"Resources/body.cmo");
	m_Obj[PARTS_COCKPIT].LoadModelFile(L"Resources/cockpit.cmo");
	m_Obj[PARTS_LAUNCHER].LoadModelFile(L"Resources/launcher.cmo");
	m_Obj[PARTS_SHIELD].LoadModelFile(L"Resources/shield.cmo");
	m_Obj[PARTS_DRILL].LoadModelFile(L"Resources/drill.cmo");

	// パーツの親子関係をセット
	m_Obj[PARTS_COCKPIT].SetParent(
		&m_Obj[PARTS_BODY]);

	m_Obj[PARTS_DRILL].SetParent(
		&m_Obj[PARTS_COCKPIT]);

	m_Obj[PARTS_LAUNCHER].SetParent(
		&m_Obj[PARTS_BODY]);

	m_Obj[PARTS_SHIELD].SetParent(
		&m_Obj[PARTS_BODY]);

	// 親からのオフセット（座標のずらし分）をセット
	m_Obj[PARTS_COCKPIT].SetTrans(
		Vector3(0, 0.37f, -0.4f));
	m_Obj[PARTS_COCKPIT].SetRot(
		Vector3(0, XM_PI, 0));

	m_Obj[PARTS_DRILL].SetTrans(
		Vector3(0, 0.1f, 0.8f));

	m_Obj[PARTS_LAUNCHER].SetTrans(
		Vector3(0, 0.37f, 0.4f));

	m_Obj[PARTS_SHIELD].SetTrans(
		Vector3(-0.8f, 0.37f, 0));
	m_Obj[PARTS_SHIELD].SetScale(
		Vector3(2, 2, 2));
	m_Obj[PARTS_SHIELD].SetRot(
		Vector3(0, 0, XM_PIDIV2));

	// 初期配置ランダム
	Vector3 pos;
	pos.x = (float)rand() / RAND_MAX * 20.0f - 10.0f;
	pos.y = 0.5f;
	pos.z = (float)rand() / RAND_MAX * 20.0f - 10.0f;
	m_Obj[0].SetTrans(pos);

	m_angle = (float)(rand() % 360);

	m_Obj[0].SetRot(Vector3(0, XMConvertToRadians(m_angle), 0));

	// 当たり判定
	m_CollisionNodeBody.Initialize();
	m_CollisionNodeBody.SetParent(&m_Obj[0]);
	m_CollisionNodeBody.SetTrans(Vector3(0, 0.3f, 0));
	m_CollisionNodeBody.SetLocalRadius(1.0f);

	m_ObjShadow.LoadModelFile(L"Resources/shadow.cmo");
	m_ObjShadow.SetTrans(Vector3(0, -0.4f, 0));
	m_ObjShadow.SetParent(&m_Obj[0]);
}

//-----------------------------------------------------------------------------
// 更新
//-----------------------------------------------------------------------------
void Enemy::Update()
{
	// 死んでいたら何もしない
	if (m_Death) return;

	// 定期的に進行方向を変える
	m_Timer--;
	if (m_Timer < 0)
	{
		m_Timer = 60;

		// 角度を変更
		float rnd = (float)rand() / RAND_MAX - 0.5f;
		rnd *= 180.0f;
		m_angle += rnd;
	}

	// じわじわと角度を反映
	{
		Vector3 rotv = m_Obj[0].GetRot();
		float angle = GetShortAngleRad(rotv.y, XMConvertToRadians(m_angle));
		rotv.y += angle*0.01f;
		m_Obj[0].SetRot(rotv);
	}

	// 機体の向いている方向に進む
	{
		// 今の座標を取得
		Vector3 trans = m_Obj[0].GetTrans();

		Vector3 move(0, 0, -0.02f);
		Vector3 rotv = m_Obj[0].GetRot();
		Matrix rotm = Matrix::CreateRotationY(rotv.y);
		move = Vector3::TransformNormal(move, rotm);
		// 座標を移動
		trans += move;
		// 移動後の座標をセット
		m_Obj[0].SetTrans(trans);
	}

	// 移動を反映して行列更新
	Calc();

	m_CollisionNodeBody.Update();
}

//-----------------------------------------------------------------------------
// 行列更新
//-----------------------------------------------------------------------------
void Enemy::Calc()
{
	// 死んでいたら何もしない
	if (m_Death) return;

	// 全パーツ分行列更新
	for (int i = 0; i < PARTS_NUM; i++)
	{
		m_Obj[i].Calc();
	}
	
	m_ObjShadow.Calc();
}

//-----------------------------------------------------------------------------
// 描画
//-----------------------------------------------------------------------------
void Enemy::Draw()
{
	// 死んでいたら何もしない
	if (m_Death) return;

	// 全パーツ分描画
	for (int i = 0; i < PARTS_NUM; i++)
	{
		m_Obj[i].Draw();
	}

	m_CollisionNodeBody.Draw();

	// 影を減算描画
	m_ObjShadow.DrawSubtractive();
}
