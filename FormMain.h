//---------------------------------------------------------------------------

#ifndef FormMainH
#define FormMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnCtrls.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ActnMan.hpp>
#include <Vcl.ActnMenus.hpp>
#include <Vcl.PlatformDefaultStyleActnCtrls.hpp>
#include <Vcl.StdActns.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.Dialogs.hpp>

#include <memory>

//---------------------------------------------------------------------------
namespace Alt {
//---------------------------------------------------------------------------

class TScrollBox : public Vcl::Forms::TScrollBox {
private:
	using inherited = Vcl::Forms::TScrollBox;

    TNotifyEvent onScroll_ { nullptr };
protected:
	virtual void __fastcall WndProc(Winapi::Messages::TMessage &Message) override {
	    inherited::WndProc( Message );
        switch ( Message.Msg ) {
            case WM_HSCROLL:
            case WM_VSCROLL:
                if ( onScroll_ ) {
                    onScroll_( this );
                }
                break;
            default:
                break;
        }
    }
public:
	__fastcall virtual TScrollBox(System::Classes::TComponent* AOwner) override
      : inherited( AOwner ) {}
    __property TNotifyEvent OnScroll = { read = onScroll_, write = onScroll_ };
};

//---------------------------------------------------------------------------
} // End of namespace Alt
//---------------------------------------------------------------------------

class TfrmMain : public TForm
{
__published:	// IDE-managed Components
    TPageControl *PageControl1;
    TTabSheet *tbshtOriginal;
    TPaintBox *paintboxOriginal;
    TTabSheet *tbshtPseudoGrey;
    TImage *imgPseudoGrey;
    TTabSheet *tbshtGrey;
    TImage *imgGrey;
    TActionManager *ActionManager1;
    TFileExit *actFileExit;
    TActionMainMenuBar *ActionMainMenuBar1;
    TAction *actFileOpen;
    TPanel *Panel1;
    TLabel *lblGreyImageInfo;
    TFileOpenDialog *FileOpenDialog1;
    Alt::TScrollBox *ScrollBox1;
    Alt::TScrollBox *ScrollBox2;
    Alt::TScrollBox *ScrollBox3;
    void __fastcall actFileOpenExecute(TObject *Sender);
    void __fastcall paintboxOriginalPaint(TObject *Sender);
    void __fastcall ImgMouseLeave(TObject *Sender);
    void __fastcall ImgMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall tbshtOriginalResize(TObject *Sender);
private:	// User declarations
    std::unique_ptr<TWICImage> img_;
    std::unique_ptr<TWICImage> scaledImg_;
    float scaleFactor_ { 1.0 };

    void LoadImage( String FileName );
    void PrepareScaledWICImage();
    void __fastcall PictureScrolled( TObject *Sender );
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
