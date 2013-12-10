/********************************************************************************/
/*                                                                              */
/*  Easy Link Library                                                           */
/*                                                                              */
/*    �ł��ȒP�ȃv���O����                                                      */
/*                                                                              */
/********************************************************************************/

#include "el.h"

// �X�N���[���̓o�^
#define	TITLE_SCREEN	2
#define	LOAD_SCREEN		3
#define	MAIN_SCREEN		4
#define DEAD_SCREEN		5
#define GOAL_SCREEN		6
#define ENDING_SCREEN	7
#define GAMEOVER_SCREEN	8

void TitleScreen(void);
void LoadScreen(void);
void MainScreen(void);
void DeadScreen(void);
void GoalScreen(void);
void EndingScreen(void);
void GameOverScreen(void);

// �֐��v���g�^�C�v
void GetInObj(FILE *fpin, char *order);
long RectInRect(RECT rc1, RECT rc2);
void AddEffectNumber(int n,int ScreenX, RECT showRc);
void DestroyAllObject();
void AddScore(int n);

//////////////////////////////////////////////////
// �N���X�E�\���̂̓o�^                         //
//////////////////////////////////////////////////

struct _POINTV{float x,y;};
////////////////////////////////////////
// �������̂̃N���X�̐��`
class Object_c
{
	// �v���C�x�[�g�ϐ�
	int		MaxPictureNumber;
	int	   *MaxCell;
	float **ChangeTimes;
	DDOBJ **ActionBMP;
	POINT **BPt;
	SIZE  **Sz;
	int JustPrevious;
	float CountShowTime;
public:
	// ���J�ϐ�
	struct _POINTV V;
	struct _POINTV JumpV;
	POINT	Pos;
	int		PictureNumber;
	int		ActionNumber;
	float	Gravity;

	// �X�C�b�`
	int		JumpSW;

	// �G�p
	int HP;
	int Dead;
	int DeadTime;
	int Side;
	int WalkTime;
	int Pattern;
	Object_c *next, *prev;

	// �A�C�e���p
	int Point;

	// ���J�֐�
	Object_c();
	void DelObject_c();
	int SetBMP(DDOBJ*, POINT*, SIZE*, int, float*);
	int ShowBMP(int, int,int);
	int GetPictMax() {return MaxPictureNumber;}
	SIZE GetNowSize();
	RECT GetRect();
	RECT HitRect();

	// �t�����h�֐�
	friend Object_c CpyObject_c(Object_c);
};

// Object_c���ł������Ɏ��s����֐�
Object_c::Object_c()
{
	// �v���C�x�[�g�ϐ��̏�����
	MaxPictureNumber = 0;
	MaxCell = 0;
	ChangeTimes = NULL;
	ActionBMP = NULL;
	BPt = NULL;
	Sz = NULL;
	JustPrevious = -1;
	CountShowTime = 0.0;

	// ���J�ϐ��̏�����
	V.x		= 0, V.y	= 0;
	Pos.x	= 0, Pos.y	= 0;
	JumpV.x = 0, JumpV.y = 0;
	JumpSW = 0;
	PictureNumber = 0;
	ActionNumber = 0;
	Gravity = 0;

	// �G�p�ϐ��̏�����
	HP = 1;
	Dead = 0;
	DeadTime = 0;
	Side = 0;
	WalkTime = 0;
	Pattern = 1;
	next = NULL;
	prev = NULL;

	// �A�C�e���p
	Point = 0;
}

// ����Object_c������
void Object_c::DelObject_c()
{
	int j;

	for( j=0; j<MaxPictureNumber; j++)
	{
		free(*(ChangeTimes+j));
		free(*(ActionBMP+j));
		free(*(BPt+j));
		free(*(Sz+j));
	}
	free(ChangeTimes);
	free(ActionBMP);
	free(BPt);
	free(Sz);
	free(MaxCell);
}

// �r�b�g�}�b�v�t�@�C�����Z�b�g����
int Object_c::SetBMP(DDOBJ *DDrawObject,POINT *Pt,SIZE *Size, int CellPic, float *CT)
{
	MaxPictureNumber++;
	
	// �V�A�h���X�����
	ActionBMP	= (DDOBJ**)realloc(ActionBMP, sizeof(DDOBJ*)*MaxPictureNumber);
	ChangeTimes	= (float**)realloc(ChangeTimes, sizeof(float*)*MaxPictureNumber);
	BPt			= (POINT**)realloc(BPt, sizeof(POINT*)*MaxPictureNumber);
	Sz			= (SIZE**)realloc(Sz, sizeof(SIZE**)*MaxPictureNumber);
	MaxCell		= (int*)realloc(MaxCell, sizeof(int)*MaxPictureNumber);

	// �|�C���^�𓖂Ă�
	*(ActionBMP+MaxPictureNumber-1) = DDrawObject;
	*(BPt+MaxPictureNumber-1) = Pt;
	*(Sz+MaxPictureNumber-1) = Size;
	*(ChangeTimes+MaxPictureNumber-1) =  CT;
	*(MaxCell+MaxPictureNumber-1) = CellPic;
	return MaxPictureNumber;
}

// x,y �� PicN�Ԗڂ̃r�b�g�}�b�v��\��
int Object_c::ShowBMP(int PicN, int x, int y)
{
	// �߂�l
	int ret = 0;

	if (MaxPictureNumber == 0)
		return -1;

	// �摜�ԍ��̏C��
	if (PicN > MaxPictureNumber)
		PicN = 1;
	PicN--;

	PictureNumber = PicN;

	if (JustPrevious != PictureNumber)
	{
		ActionNumber = 0;
		CountShowTime = 0;
	}

	JustPrevious = PictureNumber;

	// �Z���̕ύX
	CountShowTime += FrameTime * F(100);
	if (CountShowTime >= ChangeTimes[PictureNumber][ActionNumber])
	{

		ActionNumber++;
		if (ActionNumber >= MaxCell[PictureNumber])
		{
			ActionNumber = 0;
			ret = 1;
		}
		CountShowTime = 0;
	}

	// ���ۂɕ`��
	elDraw::Layer(
		x - Sz[PictureNumber][ActionNumber].cx,
		y - Sz[PictureNumber][ActionNumber].cy,
		ActionBMP[PictureNumber][ActionNumber],
		BPt[PictureNumber][ActionNumber].x,
		BPt[PictureNumber][ActionNumber].y,
		BPt[PictureNumber][ActionNumber].x + Sz[PictureNumber][ActionNumber].cx,
		BPt[PictureNumber][ActionNumber].y + Sz[PictureNumber][ActionNumber].cy
	);

	// �Z�����������ΐ^��Ԃ�
	return ret;
}

// ���ݕ\�����Ă���͈�
RECT Object_c::GetRect()
{
	RECT rc;

	rc.right = Pos.x;
	rc.bottom = Pos.y;
	rc.left = Pos.x - Sz[PictureNumber][ActionNumber].cx;
	rc.top = Pos.y - Sz[PictureNumber][ActionNumber].cy;

	return rc;
}

// �����蔻��p
RECT Object_c::HitRect()
{
	RECT rc = GetRect();

	rc.left += 2;
	rc.top += 2;
	rc.right -= 2;
	rc.bottom -= 2;

	return rc;
}

// Object_c�𕡐�
Object_c CpyObject_c(Object_c dstObject)
{
	// �߂�l
	Object_c ReObject;

	// ����
	int		MaxPictureNumber;
	int	   *MaxCell;
	float **ChangeTimes;
	DDOBJ **ActionBMP;
	POINT **BPt;
	SIZE  **Sz;
	int		Point;

	// �ėp�ϐ�
	int j,k;

	MaxPictureNumber = dstObject.MaxPictureNumber;

	MaxCell		= (int*)malloc(sizeof(int)*MaxPictureNumber);
	ChangeTimes	= (float**)malloc(sizeof(float*)*MaxPictureNumber);
	ActionBMP	= (DDOBJ**)malloc(sizeof(DDOBJ*)*MaxPictureNumber);
	BPt			= (POINT**)malloc(sizeof(POINT*)*MaxPictureNumber);
	Sz			= (SIZE**)malloc(sizeof(SIZE*)*MaxPictureNumber);
	for (j=0; j<MaxPictureNumber; j++)
	{
		MaxCell[j] = dstObject.MaxCell[j];

		ChangeTimes[j]	= (float*)malloc(sizeof(float)*MaxCell[j]);
		ActionBMP[j]	= (DDOBJ*)malloc(sizeof(DDOBJ)*MaxCell[j]);
		BPt[j]			= (POINT*)malloc(sizeof(POINT)*MaxCell[j]);
		Sz[j]			= (SIZE*)malloc(sizeof(SIZE)*MaxCell[j]);
		for (k=0; k<MaxCell[j]; k++)
		{
			ChangeTimes[j][k]	= dstObject.ChangeTimes[j][k];
			ActionBMP[j][k]		= dstObject.ActionBMP[j][k];
			BPt[j][k]			= dstObject.BPt[j][k];
			Sz[j][k]			= dstObject.Sz[j][k];
		}
	}
	Point = dstObject.Point;

	ReObject.MaxPictureNumber	= MaxPictureNumber;
	ReObject.MaxCell			= MaxCell;
	ReObject.ChangeTimes		= ChangeTimes;
	ReObject.ActionBMP			= ActionBMP;
	ReObject.BPt				= BPt;
	ReObject.Sz					= Sz;
	ReObject.Point				= dstObject.Point;
	ReObject.Pattern			= dstObject.Pattern;
	ReObject.HP					= dstObject.HP;

	return ReObject;
}

// ���݂̃T�C�Y��Ԃ�
SIZE Object_c::GetNowSize()
{
	return Sz[PictureNumber][ActionNumber];
}

////////////////////////////////////////
// �u���b�N�̈ʒu
class Block_c
{
public:
	int No;
	POINT Pos;
	POINT BPt;
	SIZE Sz;
	DDOBJ *Block;
	Block_c *next;

	Block_c(){ next = NULL;}
	RECT GetRect();
	RECT HitRect();
};

// ���݂͈̔�
RECT Block_c::GetRect()
{
	RECT rc;
	rc.left = Pos.x - Sz.cx;
	rc.top = Pos.y - Sz.cy;
	rc.right = Pos.x;
	rc.bottom = Pos.y;

	return rc;
}

// �����蔻��p
RECT Block_c::HitRect()
{
	RECT rc;

	rc.left = Pos.x - Sz.cx;
	if( No==0)	rc.top = Pos.y - (Sz.cy-12);
	else		rc.top = Pos.y - Sz.cy;
	rc.right = Pos.x;
	rc.bottom = Pos.y;

	return rc;
}

////////////////////////////////////////
// �S�[���̈ʒu
class Goal_c
{
public:
	char NextStageName[200];
	POINT Pos;
	POINT BPt;
	SIZE Sz;
	DDOBJ *Goal;
	Goal_c *next;

	Goal_c(){ next = NULL;};
	RECT GetRect();
};

//���݂͈̔�
RECT Goal_c::GetRect()
{

	RECT rc;
	
	rc.left = Pos.x - Sz.cx;
	rc.top = Pos.y - Sz.cy;
	rc.right = Pos.x;
	rc.bottom = Pos.y;

	return rc;
}

// �L�[���
static struct _Key{
	int Left;
	int Jump;
	int Right;
	int Down;
	int Dash;
	int Return;
}key;

// �O���[�o���ϐ�
Object_c Human;			// ��l���p
Object_c *Enemies;		// �G�̉摜�̐��`
Object_c StartEnemy;	// �G
Object_c *Items;		// �A�C�e���̐��`
Object_c StartItem;		// �A�C�e��
Object_c *Effect;		// �`����ʂ̐��`
Object_c StartEffect;	// ����
Block_c	StartBlock;		// �`�悷��u���b�N
Goal_c	StartGoal;		// �`�悷��S�[��

// �摜
DDOBJ	BlockBMP[2];
DDOBJ	GoalBMP;
DDOBJ	PointBMP;
DDOBJ	SDeadBMP;
DDOBJ	SGameOverBMP;
DDOBJ	SGoalBMP;
DDOBJ	TitleBMP;
DDOBJ	STitleBMP;

// midi
char	DeadMSC[50];
char	GameOverMSC[50];
char	GoalMSC[50];
char	EndingMSC[50];

//�T�E���h
DSOBJ	SelectSND;
DSOBJ	OkSND;
DSOBJ	HUMISND;
DSOBJ	JumpSND;
DSOBJ	HeadingSND;
DSOBJ	DeadSND;
DSOBJ	ItemSND;

// ���̑�
int score;
int Life;
char StageName[50];

/********************************************************************************/
/*                                                                              */
/*  ���C���֐�                                                                  */
/*                                                                              */
/********************************************************************************/
int elMain("���[��͂ǂ�����I�i�{");
{
	elDraw::SetFullColor(16,32,0);

	elLoop()
	{
		elSetScreen(LOAD_SCREEN,LoadScreen());
		elSetScreen(TITLE_SCREEN,TitleScreen());
		elSetScreen(MAIN_SCREEN,MainScreen());
		elSetScreen(DEAD_SCREEN,DeadScreen());
		elSetScreen(GOAL_SCREEN,GoalScreen());
		elSetScreen(ENDING_SCREEN,EndingScreen());
		elSetScreen(GAMEOVER_SCREEN,GameOverScreen());
	}

	elExitMain();
}

/********************************************************************************/
/*                                                                              */
/*  �E�B���h�E�����֐�                                                          */
/*                                                                              */
/********************************************************************************/
void elCreate(void)
{
	elDraw::Screen(640,480);

	// �X�v���C�g�̓����F
	elDraw::SetSpriteColor(RGB(0,0xff,0x80));

	elCallScreen(LOAD_SCREEN);
}

/********************************************************************************/
/*                                                                              */
/*  �L�[�{�[�h�֐�                                                              */
/*                                                                              */
/********************************************************************************/
void elKeyboard(void)
{
	case VK_ESCAPE:
	{
		elDraw::Exit();

		break;
	}

	elExitKeyboard();
}

/********************************************************************************/
/*                                                                              */
/*  �C�x���g�֐�                                                                */
/*                                                                              */
/********************************************************************************/
long elEvent(void)
{
	elExitEvent();
}


/********************************************************************************/
/*                                                                              */
/*  �^�C�g�����                                                                */
/*                                                                              */
/********************************************************************************/
void TitleScreen(void)
{
	static int Command;

	if( elChangeScreen() )
	{
		char FileName[150];
		char order[50];
		FILE *fp;

		sprintf(FileName,"%smarix.ini",elSystem::Directory());
		if( !(fp=fopen(FileName,"r")) )
		{
			MESG("marix.ini(�����ݒ�t�@�C��)���ǂݍ��߂܂���B");
			exit(1);
		}
		while( 1 )
		{
			GetInObj(fp, order);
			if( !strcmp(order,"stage") )
			{
				GetInObj(fp, order);
				sprintf(StageName,"%s",order);
			}
			else if( !strcmp(order,"exit-setting")||feof(fp) )
			{
				break;
			}
		}
		fclose( fp );

		Command = 0;
		Life = 10;
		score = 0;
		elMusic::Stop();
	}

	elSystem::GetKey(VK_UP, &key.Jump);
	elSystem::GetKey(VK_DOWN, &key.Down);
	elSystem::GetKey(VK_RETURN, &key.Return);

	if( key.Jump==PUSH_KEY )
	{
		if (Command>0)
			Command--;
		elSound::Play(SelectSND);
	}
	if( key.Down==PUSH_KEY )
	{
		if (Command<1)
			Command++;
		elSound::Play(SelectSND);
	}

	elDraw::Clear();

	elDraw::Layer(0,0,STitleBMP,0,0,640,480);

	elDraw::Layer(235, 300,TitleBMP,0,0,170,39);
//	elDraw::Layer(235, 350,TitleBMP,0,40,170,79);
	elDraw::Layer(235, 350,TitleBMP,0,80,170,119);

	elDraw::Layer(180, 295 + 50*Command, TitleBMP,0,120,40,160);

	elDraw::Refresh();

	if( key.Return==PUSH_KEY )
	{
		elSound::Play(OkSND);
		if(Command==0)
		{
			elCallScreen(MAIN_SCREEN);
			return;
		}
		else if(Command==1)
		{
			elDraw::Exit();
			return;
		}
	}
}

/********************************************************************************/
/*                                                                              */
/*  ���[�h���                                                                  */
/*                                                                              */
/********************************************************************************/
////////////////////////////////////////
// ���[�h��ʗp�֐�                   //
////////////////////////////////////////

////////////////////////////////////////
// ���e�̓ǂݍ���
/***************************************
 *
 * ','�����s������Ζ߂�
 * �X�y�[�X������Ε��������������ĐV���ɊJ�n
 * ';'�̓R�����g
 *
 ***************************************/
void GetInObj(FILE *fpin, char *order)
{
	int k;
	char ch;


	for (k = 0; ; k++)
	{
		ch = fgetc(fpin);
		if (ch == EOF) 
		{
			order[k] = '\0';
			return;
		}
		else if (ch == ',' || ch == '\n')
		{
			order[k] = '\0';
			return;
		}
		else if (ch == ' ')
		{
			k = -1;
			continue;
		}
		else if (ch == ';')
		{
			while (fgetc(fpin)!='\n')
				;
			order[k] = '\0';
			return;
		}
		order[k] = ch;
	}	
}

////////////////////////////////////////
// ���[�h���
void LoadScreen(void)
{
	char FileName[140];
	FILE *fp;

	// �ėp�ϐ�
	int j, k, i, n;

	elDraw::Clear();

	elDraw::Layer(190,200,elDraw::LoadObject("bmp\\load.bmp"),0,0,260,60);

	elDraw::Refresh();

	// �O���[�o���ϐ��̏�����
	Enemies	= NULL;
	Items	= NULL;
	Effect	= NULL;

	// �X�v���C�g�̓ǂݍ���
	BlockBMP[0]	 = elDraw::LoadObject("bmp\\faceblock.bmp");
	BlockBMP[1]	 = elDraw::LoadObject("bmp\\block.bmp");
	GoalBMP		 = elDraw::LoadObject("bmp\\TreasureBox.bmp");
	PointBMP	 = elDraw::LoadObject("bmp\\Point.bmp");
	SDeadBMP	 = elDraw::LoadObject("bmp\\SDead.bmp");
	SGameOverBMP = elDraw::LoadObject("bmp\\SGameOver.bmp");
	SGoalBMP	 = elDraw::LoadObject("bmp\\SGoal.bmp");
	TitleBMP	 = elDraw::LoadObject("bmp\\Title.bmp");
	STitleBMP	 = elDraw::LoadObject("bmp\\STitle.bmp");

	HUMISND		= elSound::LoadObject("sound\\Humi.wav");
	JumpSND		= elSound::LoadObject("sound\\Jump.wav");
	SelectSND	= elSound::LoadObject("sound\\select.wav");
	OkSND		= elSound::LoadObject("sound\\Ok.wav");
	HeadingSND	= elSound::LoadObject("sound\\heading.wav");
	DeadSND		= elSound::LoadObject("sound\\Dead.wav");
	ItemSND		= elSound::LoadObject("sound\\Item.wav");

	strcpy(DeadMSC, "music\\Dead.mid");
	strcpy(GameOverMSC, "music\\gameover.mid");
	strcpy(GoalMSC, "music\\Goal.mid");
	strcpy(EndingMSC, "music\\Ending.mid");

	// �����p
	sprintf(FileName, "%sconfig\\charam.cfg",elSystem::Directory());
	if (!(fp = fopen(FileName, "r")))
	{
		MESG("charam.cfg(�����p�̐ݒ�t�@�C��)��������܂���");
		exit(1);
	}
	while (1)
	{
		char order[50];

		GetInObj(fp, order);
		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		else if (!strcmp(order, "bmp"))
		{
			int num;
			float *CT;
			POINT *pt;
			SIZE *sz;
			DDOBJ *DOB;

			GetInObj(fp, order);
			num = atoi(order);

			pt = (POINT*)malloc(sizeof(POINT)*num);
			sz = (SIZE*)malloc(sizeof(SIZE)*num);
			DOB = (DDOBJ*)malloc(sizeof(DDOBJ)*num);
			CT = (float*)malloc(sizeof(float)*num);

			for (j=0; j<num; j++)
			{
				GetInObj(fp, order);
				(pt+j)->x = atoi(order);

				GetInObj(fp, order);
				(pt+j)->y = atoi(order);

				GetInObj(fp, order);
				(sz+j)->cx = atoi(order);

				GetInObj(fp, order);
				(sz+j)->cy = atoi(order);

				GetInObj(fp, order);
				*(DOB+j) = elDraw::LoadObject(order);

				GetInObj(fp, order);
				*(CT+j) = (float)atof(order);
			}
			Human.SetBMP(DOB, pt, sz, num, CT);
		}
	}
	fclose(fp);


	// �G�p
	sprintf(FileName, "%sconfig\\charae.cfg", elSystem::Directory());
	if (!(fp = fopen(FileName, "r")))
	{
		MESG("charae.cfg(�G�p�̐ݒ�t�@�C��)��������܂���B");
		exit(1);
	}
	while (1)
	{
		char order[50];

		GetInObj(fp, order);

		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		else if (!strcmp(order, "enemy"))
		{
			GetInObj(fp, order);
			j = atoi(order);

			Enemies = new Object_c[j];

			for ( k=0; k<j; k++)
			{
				GetInObj(fp, order);
				if (!strcmp(order, "film"))
				{
					GetInObj(fp, order);
					n = atoi(order);

					GetInObj(fp, order);
					Enemies[k].Point = atoi(order);

					GetInObj(fp, order);
					Enemies[k].Pattern = atoi(order);

					GetInObj(fp, order);
					Enemies[k].HP = atoi(order);

					for (; n > 0; n--)
					{
						GetInObj(fp, order);
						if (!strcmp(order, "bmp"))
						{
							int num;
							float *CT;
							POINT *pt;
							SIZE *sz;
							DDOBJ *DOB;

							GetInObj(fp, order);
							num = atoi(order);

							pt = (POINT*)malloc(sizeof(POINT)*num);
							sz = (SIZE*)malloc(sizeof(SIZE)*num);
							DOB = (DDOBJ*)malloc(sizeof(DDOBJ)*num);
							CT = (float*)malloc(sizeof(float)*num);

							for (i=0; i<num; i++)
							{
								GetInObj(fp, order);
								(pt+i)->x = atoi(order);

								GetInObj(fp, order);
								(pt+i)->y = atoi(order);

								GetInObj(fp, order);
								(sz+i)->cx = atoi(order);

								GetInObj(fp, order);
								(sz+i)->cy = atoi(order);

								GetInObj(fp, order);
								*(DOB+i) = elDraw::LoadObject(order);

								GetInObj(fp, order);
								*(CT+i) = F(atof(order));
							}
							(Enemies+k)->SetBMP(DOB, pt, sz, num, CT);
						}
					}
				}
			}	
		}
	}
	fclose(fp);


	// ���o����
	Effect = new Object_c;
	Object_c *Etmp = Effect;
	sprintf(FileName, "%sconfig\\effect.cfg", elSystem::Directory());
	if (!(fp = fopen(FileName, "r")))
	{
		MESG("effect.cfg(���o�ݒ�t�@�C��)��������܂���B");
		exit(1);
	}
	while (1)
	{
		char order[50];

		GetInObj(fp, order);

		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		if (!strcmp(order, "bmp"))
		{
			int num;
			float *CT;
			POINT *pt;
			SIZE *sz;
			DDOBJ *DOB;

			Etmp->next = new Object_c;
			Etmp = Etmp->next;

			GetInObj(fp, order);
			num = atoi(order);

			pt = (POINT*)malloc(sizeof(POINT)*num);
			sz = (SIZE*)malloc(sizeof(SIZE)*num);
			DOB = (DDOBJ*)malloc(sizeof(DDOBJ)*num);
			CT = (float*)malloc(sizeof(float)*num);

			for (i=0; i<num; i++)
			{
				GetInObj(fp, order);
				(pt+i)->x = atoi(order);

				GetInObj(fp, order);
				(pt+i)->y = atoi(order);

				GetInObj(fp, order);
				(sz+i)->cx = atoi(order);

				GetInObj(fp, order);
				(sz+i)->cy = atoi(order);

				GetInObj(fp, order);
				*(DOB+i) = PointBMP;

				GetInObj(fp, order);
				*(CT+i) = (float)atof(order);
			}
			Etmp->SetBMP(DOB, pt, sz, num, CT);
		}
	}
	fclose(fp);

	// �A�C�e��
	Items = new Object_c;
	Object_c *Itmp = Items;
	sprintf(FileName, "%sconfig\\item.cfg",elSystem::Directory());
	if (!(fp=fopen(FileName,"r")))
	{
		MESG("item.cfg(�A�C�e���ݒ�t�@�C��)���ǂݍ��߂܂���");
		exit(1);
	}
	while(1)
	{
		char order[50];

		GetInObj(fp, order);
		if (!strcmp(order, "exit-setting") || feof(fp))
		{
			break;
		}
		if (!strcmp(order, "bmp"))
		{
			int num;
			float *CT;
			POINT *pt;
			SIZE *sz;
			DDOBJ *DOB;

			Itmp->next = new Object_c;
			Itmp = Itmp->next;

			num = 1;

			pt = (POINT*)malloc(sizeof(POINT));
			sz = (SIZE*)malloc(sizeof(SIZE));
			DOB = (DDOBJ*)malloc(sizeof(DDOBJ));
			CT = (float*)malloc(sizeof(float));

			GetInObj(fp, order);
			pt->x = atoi(order);

			GetInObj(fp, order);
			pt->y = atoi(order);

			GetInObj(fp, order);
			sz->cx = atoi(order);

			GetInObj(fp, order);
			sz->cy = atoi(order);

			GetInObj(fp, order);
			*DOB = elDraw::LoadObject(order);

			GetInObj(fp, order);
			*CT = 0;

			Itmp->Point = atoi(order);

			Itmp->SetBMP(DOB, pt, sz, num, CT);
		}
	}
	fclose(fp);

	elCallScreen(TITLE_SCREEN);
}

/********************************************************************************/
/*                                                                              */
/*  ���C�����                                                                  */
/*                                                                              */
/********************************************************************************/
////////////////////////////////////////
// RECT��RECT�̏d�Ȃ蔻��
#define LEFTIN		0x1
#define TOPIN		0x2
#define RIGHTIN		0x4
#define BOTTOMIN	0x8
#define VERTICALIN	0xf
#define NEARLEFT	1
#define NEARTOP		2
#define NEARRIGHT	3
#define NEARBOTTOM	4
long RectInRect(RECT rc1, RECT rc2)
{
	POINT Pt;
	long i = 0;		// �l�����ǂ��ŐG��Ă��邩
	long e = 0;
	long t = 0;		// ���ۂɕԂ�������
	int n = 0;

	Pt.x = rc1.left;
	Pt.y = rc1.top;
	if( PtInRect(&rc2,Pt) )
	{
		i |= TOPIN | LEFTIN;
	}
	Pt.y = rc1.bottom;
	if( PtInRect(&rc2,Pt) )
	{
		i |= BOTTOMIN | LEFTIN;
	}
	Pt.x = rc1.right;
	Pt.y = rc1.top;
	if( PtInRect(&rc2,Pt) )
	{
		i |= TOPIN | RIGHTIN;
	}
	Pt.y = rc1.bottom;
	if( PtInRect(&rc2,Pt) )
	{
		i |= BOTTOMIN | RIGHTIN;
	}

	Pt.x = rc2.left;
	Pt.y = rc2.top;
	if( PtInRect(&rc1,Pt) )
	{
		e |= BOTTOMIN | RIGHTIN;
	}
	Pt.y = rc2.bottom;
	if( PtInRect(&rc1,Pt) )
	{
		e |= TOPIN | RIGHTIN;
	}
	Pt.x = rc2.right;
	Pt.y = rc2.top;
	if( PtInRect(&rc1,Pt) )
	{
		e |= BOTTOMIN | LEFTIN;
	}
	Pt.y = rc2.bottom;
	if( PtInRect(&rc1,Pt) )
	{
		e |= TOPIN | LEFTIN;
	}

	t = i|e;

	return t;
}

////////////////////////////////////////
// �����̂�\��
void AddEffectNumber(int n,int ScreenX, RECT showRc)
{
	for (int line=0; n>0;line++)
	{
		Object_c *Eftmp = new Object_c;
		Object_c *Eftmp2 = Effect->next;
		int c;
		
		for (c=n%10; c>0; c--)
			Eftmp2 = Eftmp2->next;

		*Eftmp = CpyObject_c(*Eftmp2);
		Eftmp->Pos.x = showRc.right + ScreenX - line*16;
		Eftmp->Pos.y = showRc.bottom;

		if (StartEffect.next!=NULL)
			StartEffect.next->prev = Eftmp;
		Eftmp->next = StartEffect.next;
		StartEffect.next = Eftmp;
		Eftmp->prev = &StartEffect;
		n /= 10;
	}
}

////////////////////////////////////////
// �X�R�A�𑝉�
void AddScore(int n)
{
	static int Pscore=0;

	score += n;
	if( score-Pscore>5000 )
	{
		Pscore = score;
		Life++;
	}
}

////////////////////////////////////////
// �S�Ă�����
void DestroyAllObject()
{
	Object_c *Otmp1, *Otmp2;
	Block_c *Btmp1, *Btmp2;
	Goal_c *Gtmp1, *Gtmp2;


	// �G
	Otmp1 = StartEnemy.next;
	StartEnemy.next = NULL;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		Otmp1->DelObject_c();
		free(Otmp1);
		Otmp1 = Otmp2;
	}

	// �A�C�e��
	Otmp1 = StartItem.next;
	StartItem.next = NULL;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		Otmp1->DelObject_c();
		free(Otmp1);
		Otmp1 = Otmp2;
	}

	// ���o����
	Otmp1 = StartEffect.next;
	StartEffect.next = NULL;
	while( Otmp1!=NULL )
	{
		Otmp2 = Otmp1->next;
		Otmp1->DelObject_c();
		free(Otmp1);
		Otmp1 = Otmp2;
	}

	// �u���b�N
	Btmp1 = StartBlock.next;
	StartBlock.next = NULL;
	while( Btmp1!=NULL )
	{
		Btmp2 = Btmp1->next;
		free(Btmp1);
		Btmp1 = Btmp2;
	}

	// �S�[��
	Gtmp1 = StartGoal.next;
	StartGoal.next = NULL;
	while( Gtmp1!=NULL )
	{
		Gtmp2 = Gtmp1->next;
		free(Gtmp1);
		Gtmp1 = Gtmp2;
	}
	elDraw::Clear();
}


////////////////////////////////////////
// ���C�����

// ���C���X�N���[��
void MainScreen(void)
{
	// �����p
	static POINT StartPt;
	static int PicN = 1;
	static int CommandTime;

	// �X�e�[�W�ݒ�p
	const int	HumanStand = 1,
				HumanRight = 2,
				HumanLeft = 3,
				HumanUp = 4,
				HumanDown = 5;
	static float ohoho = 0;
	static int ScreenX = 0;
	static int ScreenMax = 640;
	static int Time;
	static clock_t ct1;

	// �����蔻��p
	int overlap = 0;
	int LevelDirection = 0,
		VerticalDirection = 0;

	Object_c *Etmp;		// �G
	Object_c *Itmp;		// �A�C�e��
	Object_c *Eftmp;	// ����
	Block_c *Btmp;		// �u���b�N
	Goal_c *Gtmp;		// �S�[��


	// �ėp�ϐ�
	int j, k, i;
	char s[50];

	/***************************************
	 * ������
	 ***************************************/
	if (elChangeScreen())
	{
		char FileName[140];
		char order[50];
		FILE *fp;

		// �ϐ��̏�����
		ScreenX = 0;
		ct1 = clock();
		srand((unsigned int)time(NULL));
		Human.V.x = 0;
		Human.V.y = 0;
		Human.JumpV.x = 0;
		Human.JumpV.y = 0;
		Human.JumpSW = 0;
		Human.Gravity = 0;

		// �X�e�[�W�̍쐬
		sprintf(FileName,"%s%s",elSystem::Directory(),StageName);
		if (!(fp = fopen(FileName, "r")))
		{
			elDraw::Exit();
			return;
		}
		while (1)
		{
			GetInObj(fp, order);
			// �u���b�N�̏�����
			if (!strcmp(order, "block"))
			{
				GetInObj(fp, order);
				k = atoi(order);

				Btmp = &StartBlock;
				for (j=0; j<k; j++)
				{
					Btmp = 	(Btmp->next = new Block_c);
	
					GetInObj(fp, order);
					Btmp->No = atoi(order)-1;
					Btmp->Block = &BlockBMP[Btmp->No];

					GetInObj(fp, order);
					Btmp->Pos.x = atoi(order)+32;
	
					GetInObj(fp, order);
					Btmp->Pos.y = atoi(order)+32;
	
					Btmp->BPt.x = 0;
					Btmp->BPt.y = 0;
					Btmp->Sz.cx = 32;
					Btmp->Sz.cy = 32;
				}
			}
			// �S�[���̏�����
			else if (!strcmp(order, "goal"))
			{
				GetInObj(fp, order);
				k = atoi(order);

				Gtmp = &StartGoal;
				for ( j=0; j<k; j++)
				{
					Gtmp->next = new Goal_c;
					Gtmp = Gtmp->next;
					Gtmp->next = NULL;

					GetInObj(fp, order);
					Gtmp->Pos.x = atoi(order)+32;

					GetInObj(fp, order);
					Gtmp->Pos.y = atoi(order)+32;

					Gtmp->BPt.x = 0;
					Gtmp->BPt.y = 0;
					Gtmp->Sz.cx = 32;
					Gtmp->Sz.cy = 32;

					GetInObj(fp, order);
					sprintf(Gtmp->NextStageName,"%s",order);

					Gtmp->Goal = &GoalBMP;
				}
			}
			// �A�C�e��
			else if (!strcmp(order, "item"))
			{
				GetInObj(fp, order);
				k = atoi(order);

				Itmp = &StartItem;
				for (j=0; j<k; j++)
				{
					Object_c *Itmp2;
					Itmp->next = new Object_c;

					GetInObj(fp, order);
					i = atoi(order);

					for (Itmp2 = Items;i>0;i--,Itmp2 = Itmp2->next);
						
					*Itmp->next = CpyObject_c(*Itmp2);
					Itmp->next->prev = Itmp;
					Itmp = Itmp->next;

					GetInObj(fp, order);
					Itmp->Pos.x = atoi(order) + 32;

					GetInObj(fp, order);
					Itmp->Pos.y = atoi(order) + 32;
				}
			}
			// �G�̏�����
			else if (!strcmp(order, "enemy"))
			{
				GetInObj(fp, order);
				k = atoi(order);

				Etmp = &StartEnemy;
				for (j=0; j<k; j++)
				{
					Etmp->next = new Object_c;

					GetInObj(fp, order);
					i = atoi(order) - 1;

					*Etmp->next = CpyObject_c(Enemies[i]);

					Etmp->next->prev = Etmp;
					Etmp = Etmp->next;

					GetInObj(fp, order);
					Etmp->Pos.x = atoi(order) + Etmp->GetNowSize().cx;

					GetInObj(fp, order);
					Etmp->Pos.y = atoi(order) + Etmp->GetNowSize().cy;

					if( Etmp->Pattern==2 )
					{
						Etmp->JumpV.y = -8;
						Etmp->Side = ((rand()%2)==(1)?(-1):(1));
					}
				}

			}
			// �����̏�����
			else if (!strcmp(order, "human"))
			{
				GetInObj(fp, order);
				Human.Pos.x = atoi(order)+32;
				StartPt.x = Human.Pos.x;
				GetInObj(fp, order);
				Human.Pos.y = atoi(order)+32;
				StartPt.y = Human.Pos.y;
			}
			// �~���[�W�b�N�𗬂�
			else if( !strcmp(order,"music") )
			{
				elMusic::Stop();
				GetInObj(fp, order);
				elMusic::Play(order);
			}
			// �X�N���[���̈ʒu
			else if( !strcmp(order,"screenx") )
			{
				GetInObj(fp, order);
				ScreenX = atoi(order);
			}
			// �X�N���[���̍ő�ʒu
			else if( !strcmp(order,"screenmax") )
			{
				GetInObj(fp, order);
				ScreenMax = atoi(order);
			}
			// ���Ԑ���
			else if( !strcmp(order,"timer") )
			{
				GetInObj(fp, order);
				Time = atoi(order);
			}
			// �ݒ芮��
			else if (!strcmp(order, "exit-setting"))
			{
				fclose(fp);
				break;
			}
		}

		return;
	}

	/***************************************
	 * �L�[����
	 ***************************************/
	key.Jump = FREE_KEY;
	elSystem::GetKey(VK_LEFT, &key.Left);
	elSystem::GetKey(VK_UP, &key.Jump);
	elSystem::GetKey(VK_RIGHT, &key.Right);
	elSystem::GetKey(VK_DOWN, &key.Down);
	elSystem::GetKey(VK_CONTROL, &key.Dash);

	if (key.Left != FREE_KEY)
	{
		Human.V.x -= 3;
		if (key.Dash != FREE_KEY)
			Human.V.x -= 2;
		PicN = HumanLeft;	
	}
	if (key.Right != FREE_KEY)
	{
		Human.V.x += 3;
		if (key.Dash != FREE_KEY)
			Human.V.x += 2;
		PicN = HumanRight;
	}
	if (key.Jump != FREE_KEY)
	{
		if (Human.JumpSW == 0)
		{
			Human.JumpV.x = Human.V.x;
			Human.JumpV.y = -11;
			Human.JumpSW = 1;
			CommandTime = 20;
			elSound::Play(JumpSND);
		}
		if (Human.JumpSW == 1 && CommandTime <= 0)
		{
			Human.JumpV.x = Human.V.x;
			Human.JumpV.y = -9;
			Human.Gravity = 0;
			Human.JumpSW = 2;
			elSound::Play(JumpSND);
		}
	}
	if (key.Down != FREE_KEY)
	{
		Human.V.y += 2;
		if (!Human.JumpSW)
			PicN = HumanDown;
	}
	if (!(key.Left != FREE_KEY ||
		key.Jump != FREE_KEY ||
		key.Right != FREE_KEY || 
		key.Down != FREE_KEY) )
	{
		if (!Human.JumpSW)
			PicN = HumanStand;
	}


	/***************************************
	 * ���Ԗ��Ȃǂł̈ړ��E�����Ȃ�
	 ***************************************/
	// �^�C�}�[
	if( (clock() - ct1)/CLOCKS_PER_SEC>=1 )
	{
		ct1 = clock();
		Time--;
		if( Time<0 )
		{
			Life--;
			DestroyAllObject();
			elCallScreen(DEAD_SCREEN);
			return;
		}
	}

	// �W�����v�̎���
	if (CommandTime)
		CommandTime--;

	// �d��
	Human.Gravity += F(0.5);
	if (Human.Gravity < 1)
		Human.Gravity = F(1.0);
	Human.V.y += (int)(Human.Gravity + Human.JumpV.y);

	Etmp = StartEnemy.next;
	while (Etmp!=NULL)
	{
		/*
		if ((Etmp->GetRect().left - ScreenX > 640 ||
			Etmp->GetRect().right < ScreenX))
		{
			Etmp = Etmp->next;
			continue;
		}
		*/
		Etmp->Gravity += F(0.3);
		if (Etmp->Gravity < 1)
			Etmp->Gravity = F(1.0);
		Etmp->V.y += (int)(Etmp->Gravity + Etmp->JumpV.y);

		Etmp = Etmp->next;
	}

	// �G������ł��������
	Etmp = StartEnemy.next;
	while( Etmp!=NULL )
	{
		if (Etmp->Dead==1)
		{
			Object_c *Etmp2 = Etmp->next;
			Etmp = Etmp->prev;
			Etmp->next->DelObject_c();
			free(Etmp->next);
			if (Etmp2!=NULL)
				Etmp2->prev = Etmp;
			Etmp->next = Etmp2;
			Etmp = Etmp->next;
			continue;
		}
		Etmp = Etmp->next;
	}

	// ���o���ʂ̎��Ԃ��؂ꂽ�����
	Eftmp = StartEffect.next;
	while( Eftmp!=NULL )
	{
		if (Eftmp->Dead == 1)
		{
			Object_c *Eftmp2 = Eftmp->next;
			Eftmp = Eftmp->prev;
			Eftmp->next->DelObject_c();
			free(Eftmp->next);
			Eftmp->next = Eftmp2;
			if (Eftmp2!=NULL)
				Eftmp2->prev = Eftmp;
		}
		Eftmp = Eftmp->next;
	}

	// �A�C�e�����擾���������
	Itmp = StartItem.next;
	while( Itmp!=NULL )
	{
		if( Itmp->Dead==1 )
		{
			Object_c *Itmp2 = Itmp->next;
			Itmp = Itmp->prev;
			Itmp->next->DelObject_c();
			free(Itmp->next);
			Itmp->next = Itmp2;
			if (Itmp2!=NULL)
				Itmp2->prev = Itmp;
		}
		Itmp = Itmp->next;
	}

	/***************************************
	 * �����蔻��
	 ***************************************/
	// �u���b�N��
	Btmp = StartBlock.next;
	overlap = 0;
	while (Btmp != NULL)
	{
		if (Btmp->GetRect().left - ScreenX > 640 ||
			Btmp->GetRect().right < ScreenX)
		{
			Btmp = Btmp->next;
			continue;
		}
		RECT MyRc, dstRc;
		MyRc = Human.HitRect();
		dstRc = Btmp->HitRect();

		// ���ݒ�
		dstRc.left -= ScreenX;
		dstRc.right -= ScreenX;

		// �������̓����蔻��
		MyRc.left += (long)Human.V.x;
		MyRc.right += (long)Human.V.x;
		LevelDirection = RectInRect(MyRc,dstRc);

		// �������̓����蔻��
		MyRc = Human.HitRect();
		MyRc.top += (long)Human.V.y;
		MyRc.bottom += (long)Human.V.y;
		VerticalDirection = RectInRect(MyRc, dstRc);

		overlap = overlap | (LevelDirection & 0x1) | (LevelDirection & 0x4);
		if( LevelDirection&0x1 )
		{
			Human.Pos.x = dstRc.right + Human.GetNowSize().cx;
		}
		if( LevelDirection&0x4 )
		{
			Human.Pos.x = dstRc.left;
		}

		overlap = overlap | (VerticalDirection & 0x2) | (VerticalDirection & 0x8);
		if( VerticalDirection&0x2 )
		{
			Human.Pos.y = dstRc.bottom + Human.GetNowSize().cy;
			elSound::Stop(JumpSND);
			elSound::Play(HeadingSND);
		}
		if( VerticalDirection&0x8 )
		{
		}

		Btmp = Btmp->next;
	}
	if (overlap & 0x1)
	{
		Human.V.x = 0;
	}
	if (overlap & 0x4)
	{
		Human.V.x = 0;
	}
	if (overlap & 0x2)
	{
		Human.V.y = 0;
		Human.Gravity = 0;
		Human.JumpV.y = 3;
	}
	if (overlap & 0x8)
	{
		Human.V.y = 0;
		Human.Gravity = 0;
		Human.JumpSW = 0;
		Human.JumpV.y = 0;
	}

	// �G�Ƃ̂����蔻��
	Etmp = StartEnemy.next;
	overlap = 0;
	while (Etmp != NULL)
	{
		if( Etmp->Dead )
		{
			Etmp = Etmp->next;
			continue;
		}
		if ((Etmp->GetRect().left - ScreenX > 640 ||
			Etmp->GetRect().right < ScreenX))
		{
			Etmp = Etmp->next;
			continue;
		}
		RECT MyRc, dstRc;
		MyRc = Human.HitRect();
		dstRc = Etmp->GetRect();

		// ���ݒ�
		dstRc.left -= ScreenX;
		dstRc.right -= ScreenX;
		MyRc.left += (long)Human.V.x;
		MyRc.right += (long)Human.V.x;
		MyRc.top += (long)Human.V.y;
		MyRc.bottom += (long)Human.V.y;

		// �������̂����蔻��
		LevelDirection = RectInRect(MyRc,dstRc);
		overlap = overlap | (LevelDirection & 0x1) | (LevelDirection & 0x4);
		// ���ŐG�ꂽ
		if( LevelDirection & 0x1 )
		{
		}
		// �E�ŐG�ꂽ
		if( LevelDirection & 0x4 )
		{
		}

		// �c�����̓����蔻��
		VerticalDirection = RectInRect(MyRc, dstRc);
		overlap = overlap | (VerticalDirection & 0x2) | (VerticalDirection & 0x8);
		// ��ŐG�ꂽ
		if( VerticalDirection & 0x2 )
		{
		}
		// ���ŐG�ꂽ
		if( VerticalDirection & 0x8 )
		{
			Etmp->HP--;
			if( Etmp->HP<=0 )
			{
				Etmp->Dead = -1;
				AddScore(Etmp->Point);
				AddEffectNumber(Etmp->Point,ScreenX, dstRc);
			}
			elSound::Play(HUMISND);

			CommandTime = 20;
		}

		// ���S����
		if( LevelDirection||VerticalDirection)
		{
			if( (dstRc.bottom-(dstRc.bottom-dstRc.top)/4) < MyRc.bottom)
			{
				elSound::Play(DeadSND);
				Life--;
				DestroyAllObject();
				elCallScreen(DEAD_SCREEN);
				return;
			}
		}
		Etmp = Etmp->next;
	}
	if (overlap & 0x8)
	{
		Human.V.y = 0;
		Human.JumpSW = 1;
		Human.JumpV.y = -9;
		Human.Gravity = 0.0;
	}

	// �A�C�e���Ƃ�����
	Itmp = StartItem.next;
	overlap = 0;
	while( Itmp!=NULL )
	{
		if (Itmp->GetRect().left - ScreenX > 640 ||
			Itmp->GetRect().right < ScreenX)
		{
			Itmp = Itmp->next;
			continue;
		}
		RECT MyRc, dstRc;
		MyRc = Human.HitRect();
		dstRc = Itmp->GetRect();

		// ���ݒ�
		dstRc.left -= ScreenX;
		dstRc.right -= ScreenX;

		overlap = RectInRect(MyRc, dstRc);
		if (LOWORD(overlap))
		{
			Itmp->Dead = 1;
			AddScore(Itmp->Point);
			elSound::Play(ItemSND);
			AddEffectNumber(Itmp->Point,ScreenX,dstRc);
		}
		Itmp = Itmp->next;
	}

	// �S�[���Ƃ̂����蔻��
	Gtmp = StartGoal.next;
	overlap = 0;
	while( Gtmp!=NULL )
	{
		if (Gtmp->GetRect().left - ScreenX > 640 ||
			Gtmp->GetRect().right < ScreenX)
		{
			Gtmp = Gtmp->next;
			continue;
		}
		RECT MyRc, dstRc;
		MyRc = Human.HitRect();
		dstRc = Gtmp->GetRect();

		// ���ݒ�
		dstRc.left -= ScreenX;
		dstRc.right -= ScreenX;

		overlap = RectInRect(MyRc, dstRc);
		if (LOWORD(overlap))
		{
			strcpy(StageName,Gtmp->NextStageName);
			DestroyAllObject();
			elCallScreen(GOAL_SCREEN);
			return;
		}
		Gtmp = Gtmp->next;
	}

	// �����̈ړ�
	if (Human.Gravity > 1)
	{
		if (!Human.JumpSW)
			Human.JumpSW = 1;
	}
	Human.Pos.x += (long)Human.V.x;
	Human.Pos.y += (long)Human.V.y;
	Human.V.x = 0;
	Human.V.y = 0;


	// �G�̈ړ�
	Etmp = StartEnemy.next;
	while( Etmp != NULL)
	{
		if( Etmp->Dead )
		{
			Etmp = Etmp->next;
			continue;
		}

		// �u���b�N�Ƃ̓�����
		Btmp = StartBlock.next;
		overlap = 0;
		LevelDirection = 0;
		VerticalDirection = 0;
		while (Btmp != NULL)
		{
			if ((Btmp->GetRect().left > Etmp->GetRect().right ||
				Btmp->GetRect().right < Etmp->GetRect().left))
			{
				Btmp = Btmp->next;
				continue;
			}

			RECT MyRc, dstRc;
			MyRc = Etmp->HitRect();
			dstRc = Btmp->HitRect();

			// ���ݒ�
			// ���͓��ɂȂ�

			// �������̓����蔻��
			MyRc.left += (long)Etmp->V.x;
			MyRc.right += (long)Etmp->V.x;
			
			LevelDirection = RectInRect(MyRc,dstRc);
			overlap = overlap | (LevelDirection & 0x1) | (LevelDirection & 0x4);
			if( LevelDirection&0x1 )
			{
				Etmp->Pos.x += 4;
			}
			if( LevelDirection&0x4 )
			{
				Etmp->Pos.x -= 4;
			}

			// �������̓����蔻��
			MyRc = Etmp->HitRect();
			MyRc.top += (long)Etmp->V.y;
			MyRc.bottom += (long)Etmp->V.y;

			VerticalDirection = RectInRect(MyRc, dstRc);
			overlap = overlap | (VerticalDirection & 0x2) | (VerticalDirection & 0x8);
			if (VerticalDirection & 0x2)
			{
			}
			if (VerticalDirection & 0x8)
			{
				Etmp->Pos.y = dstRc.top+1;
			}
			Btmp = Btmp->next;
		}
		// ��
		if (overlap & 0x1)
		{
			Etmp->Side = 1;
		}
		// �E
		if (overlap & 0x4)
		{
			Etmp->Side = -1;
		}
		// ��
		if (overlap & 0x2)
		{
			Etmp->Gravity = -Etmp->JumpV.y;
			Etmp->Pos.y += 10;
		}
		if (overlap & 0x8)
		{
			Etmp->V.y = 0;
			Etmp->V.x = F(1.5 * Etmp->Side);
			Etmp->Gravity = 0;
			Etmp->JumpSW = 0;
			if (!Etmp->Side)
			{
				Etmp->Side = ((rand()%2)==(1)?(1):(-1));
			}
		}
		else
		{
			if (Etmp->Side)
			{
				Etmp->V.x = F(1.5 * Etmp->Side);
				Etmp->JumpSW = 0;
			}
		}
		
		// �G�̈ړ�������
		Etmp->Pos.x += (long)Etmp->V.x;
		Etmp->Pos.y += (long)Etmp->V.y;
		Etmp->V.x = 0;
		Etmp->V.y = 0;
		if( Etmp->Pos.y>480 )
		{
			Etmp->Pos.y = 0;
			Etmp->Gravity = 0.0;
		}
		if( Etmp->GetRect().left<0 )
		{
			Etmp->Side *= -1;
		}

		Etmp = Etmp->next;
	}
	/***************************************
	 * ��ʂ̐ݒ�
	 ***************************************/
	
	if (Human.Pos.x >= 440)
	{
		ScreenX += Human.Pos.x - 440;
		if( ScreenMax > ScreenX + 640 )
		{
			Human.Pos.x -= Human.Pos.x - 440;
		}
		else
		{
			ScreenX = ScreenMax - 640;
		}
	}
	if (Human.Pos.x < 200)
	{
		ScreenX -= 200 - Human.Pos.x;
		if( ScreenX > 0 )
		{
			Human.Pos.x += 200 - Human.Pos.x;
		}
		if( ScreenX < 0 )
		{
			ScreenX = 0;
		}
	}
	if (Human.Pos.y > 480)
	{
		elSound::Play(DeadSND);
		Life--;
		DestroyAllObject();
		elCallScreen(DEAD_SCREEN);
		return;
	}
	
	if( Human.GetRect().left < 0 )
		Human.Pos.x = Human.GetNowSize().cx;
	if( Human.GetRect().right > 640 )
		Human.Pos.x = 640;

	/***************************************
	 * �`��
	 ***************************************/
	elDraw::Clear();
	elDraw::ColorFill(0,0,640,480,RGB16(0,0x55,0xff));


	// �A�C�e��
	Itmp = StartItem.next;
	while( Itmp!=NULL )
	{
		if (Itmp->GetRect().left - ScreenX > 640 ||
			Itmp->GetRect().right < ScreenX)
		{
			Itmp = Itmp->next;
			continue;
		}

		Itmp->ShowBMP(1,Itmp->Pos.x-ScreenX,Itmp->Pos.y);
		Itmp=Itmp->next;
	}

	// �S�[��
	Gtmp=StartGoal.next;
	while( Gtmp!=NULL )
	{
		if (Gtmp->GetRect().left - ScreenX > 640 ||
			Gtmp->GetRect().right < ScreenX)
		{
			Gtmp = Gtmp->next;
			continue;
		}

		elDraw::Layer(
			Gtmp->GetRect().left - ScreenX,
			Gtmp->GetRect().top,
			*Gtmp->Goal,
			Gtmp->BPt.x,
			Gtmp->BPt.y,
			Gtmp->Sz.cx,
			Gtmp->Sz.cy
		);
		Gtmp=Gtmp->next;
	}

	// �G
	Etmp=StartEnemy.next;
	while ( Etmp!=NULL )
	{
		if ((Etmp->GetRect().left - ScreenX > 640 ||
			Etmp->GetRect().right < ScreenX))
		{
			Etmp = Etmp->next;
			continue;
		}

		if (Etmp->Dead==-1)
		{
			Etmp->ShowBMP((int)(3.5+0.5*Etmp->Side),Etmp->Pos.x-ScreenX,Etmp->Pos.y);
			Etmp->DeadTime++;
			if (Etmp->DeadTime > 30)
				Etmp->Dead = 1;
		}
		else
		{
			Etmp->ShowBMP((int)(1.5+0.5*Etmp->Side),Etmp->Pos.x-ScreenX,Etmp->Pos.y);
		}
		Etmp=Etmp->next;
	}

	// ����
	Human.ShowBMP(PicN, Human.Pos.x, Human.Pos.y);

	// �u���b�N
	Btmp=StartBlock.next;
	while( Btmp!=NULL )
	{
		if (Btmp->GetRect().left - ScreenX > 640 ||
			Btmp->GetRect().right < ScreenX)
		{
			Btmp = Btmp->next;
			continue;
		}
		elDraw::Layer(
			Btmp->GetRect().left - ScreenX,
			Btmp->GetRect().top,
			*Btmp->Block,
			Btmp->BPt.x,
			Btmp->BPt.y,
			Btmp->Sz.cx,
			Btmp->Sz.cy
		);
		Btmp=Btmp->next;
	}

	// ���o����
	Eftmp = StartEffect.next;
	while( Eftmp!=NULL )
	{
		if (Eftmp->ShowBMP(1,Eftmp->Pos.x - ScreenX, Eftmp->Pos.y))
		{
			Eftmp->Dead = 1;
		}
		Eftmp = Eftmp->next;
	}

	// �t�H���g�̕`��J�n
	elFont::Begin("Comic Sans MS Bold",16);
	elFont::Color(RGB(0x55,0x55,0x80),RGB(120,0,80),TRUE);

	sprintf(s,"�X�R�A�@%d", score);
	elFont::Draw(0,0,s);
	
	sprintf(s,"�c�� %d�C", Life-1);
	elFont::Draw(100,0,s);

	sprintf(s,"�c�莞�� %d.%d", Time,(clock()-ct1)%1000/10);
	elFont::Draw(200,0,s);


	// �t�H���g�`��I��
	elFont::Before();

	// ���`��I��
	elDraw::Refresh();
}

/********************************************************************************/
/*                                                                              */
/*  �������ꂽ���                                                            �@*/
/*                                                                              */
/********************************************************************************/
void DeadScreen(void)
{
	if( elChangeScreen() )
	{
		key.Return = FREE_KEY;
		if (Life<=0)
			elCallScreen(GAMEOVER_SCREEN);
		elMusic::Stop();
		elMusic::Play(DeadMSC,FALSE);
		return;
	}

	elSystem::GetKey(VK_RETURN, &key.Return);
	
	elDraw::Clear();

	elDraw::Layer(0,0,SDeadBMP,0,0,640,480);

	elDraw::Refresh();
	if( key.Return==PUSH_KEY )
		elCallScreen(MAIN_SCREEN);
}

/********************************************************************************/
/*                                                                              */
/*  �S�[�����                                                                  */
/*                                                                              */
/********************************************************************************/
void GoalScreen(void)
{
	if( elChangeScreen() )
	{
		key.Return = FREE_KEY;
		if( !strcmp(StageName,"NULL") )
		{
			elCallScreen(ENDING_SCREEN);
			return;
		}
		elMusic::Stop();
		elMusic::Play(GoalMSC,FALSE);
	}
	elSystem::GetKey(VK_RETURN, &key.Return);

	elDraw::Clear();

	elDraw::Layer(0,0,SGoalBMP,0,0,640,480);

	elDraw::Refresh();

	if( key.Return==PUSH_KEY )
		elCallScreen(MAIN_SCREEN);
}

/********************************************************************************/
/*                                                                              */
/*  �Q�[���I�[�o�[���                                                          */
/*                                                                              */
/********************************************************************************/
void GameOverScreen()
{
	if( elChangeScreen() )
	{
		key.Return = FREE_KEY;
		elMusic::Stop();
		elMusic::Play(GameOverMSC,FALSE);
	}
	elSystem::GetKey(VK_RETURN, &key.Return);

	elDraw::Clear();

	elDraw::Layer(0,0,SGameOverBMP,0,0,640,480);

	elDraw::Refresh();
	if( key.Return==PUSH_KEY )
		elCallScreen(TITLE_SCREEN);
}

/********************************************************************************/
/*                                                                              */
/*  �G���f�B���O���                                                            */
/*                                                                              */
/********************************************************************************/
void EndingScreen(void)
{
	int x, y;
	static int ss;
	static int scroll;
	static char **s;
	if( elChangeScreen() )
	{
		char FileName[150];
		char ch[100];
		FILE *fp;

		sprintf(FileName,"%sending.tlp",elSystem::Directory());
		if( !(fp=fopen(FileName,"rw")) )
		{
			elCallScreen(TITLE_SCREEN);
			return;
		}

		scroll=0;
		ss = 0;
		s = NULL;
		while(!feof(fp))
		{
			fgets(ch,100,fp);
			s = (char**)realloc(s,sizeof(char*)*(ss+1));
			*(s+ss) = (char*)malloc(strlen(ch)+1);
			strcpy(*(s+ss),ch);
			ss++;
		}
		fclose(fp);

		key.Return = FREE_KEY;
		elMusic::Stop();
		elMusic::Play(EndingMSC,FALSE);
	}

	x = y = 0;

	elSystem::GetKey(VK_RETURN, &key.Return);

	elDraw::Clear();

	elFont::BeginAA("�l�r �S�V�b�N",24);
	elFont::Color(RGB(255,255,255));
	for( int k=0;k<ss;k++ )
	{
		x = (int)((640 - strlen(*(s+k))*12)/2);
		elFont::DrawAA(x,480+y-scroll,x+strlen(*(s+k))*24,480+y+24-scroll,*(s+k));
		y += 24;

	}
	if(480+y-scroll<0)
	{
		x = (int)((640 - strlen("�A�q�� ��")*12)/2);
		elFont::DrawAA(x,230,640,230+24,"�A�q���@��");
	}
	elFont::BeforeAA();

	elDraw::Refresh();

	if( key.Return==PUSH_KEY )
		elCallScreen(TITLE_SCREEN);

	scroll+=3;
}
