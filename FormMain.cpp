//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <algorithm>
#include <array>

#include <System.Win.ComObj.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Direct2D.hpp>

#include "FormMain.h"

using std::make_unique;
//using std::min;
using std::clamp;
using std::array;

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------

__fastcall TfrmMain::TfrmMain(TComponent* Owner)
    : TForm(Owner)
{
    imgGrey->ControlStyle = imgGrey->ControlStyle << csOpaque;
    imgPseudoGrey->ControlStyle = imgPseudoGrey->ControlStyle << csOpaque;
    paintboxOriginal->ControlStyle = paintboxOriginal->ControlStyle << csOpaque;
    tbshtOriginal->ControlStyle = tbshtOriginal->ControlStyle << csOpaque;
    ScrollBox1->OnScroll = PictureScrolled;
    ScrollBox2->OnScroll = PictureScrolled;
    ScrollBox3->OnScroll = PictureScrolled;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::actFileOpenExecute(TObject *Sender)
{
    if ( FileOpenDialog1->Execute() ) {
        LoadImage( FileOpenDialog1->FileName );
    }
}
//---------------------------------------------------------------------------

void TfrmMain::LoadImage( String FileName )
{
    auto Img = make_unique<TWICImage>();
    Img->LoadFromFile( FileName );
    _di_IWICBitmap IBmp = Img->Handle;

    const int ImgWidth = Img->Width;
    const int ImgHeight = Img->Height;
    const WICRect rcLock = { 0, 0, ImgWidth, ImgHeight };
    _di_IWICBitmapLock pLock;
    OleCheck( IBmp->Lock( &rcLock, WICBitmapLockRead, &pLock ) );
    WICPixelFormatGUID PixelFormat;
    Comobj::OleCheck( pLock->GetPixelFormat( &PixelFormat ) );
    if ( PixelFormat == GUID_WICPixelFormat16bppGray ) {
        imgGrey->Width = ImgWidth;
        imgGrey->Height = ImgHeight;

        imgPseudoGrey->Width = ImgWidth;
        imgPseudoGrey->Height = ImgHeight;

        paintboxOriginal->ClientWidth = ImgWidth;
        paintboxOriginal->ClientHeight = ImgHeight;

        auto BmpPseudoGray = make_unique<TBitmap>();
        BmpPseudoGray->PixelFormat = pf24bit;
        BmpPseudoGray->Width = ImgWidth;
        BmpPseudoGray->Height = ImgHeight;

        auto BmpGray = make_unique<TBitmap>();
        BmpGray->PixelFormat = pf24bit;
        BmpGray->Width = ImgWidth;
        BmpGray->Height = ImgHeight;

        UINT BufferSize = 0;
        UINT Stride = 0;
        BYTE *Data = nullptr;
        OleCheck( pLock->GetDataPointer( &BufferSize, &Data ) );
        OleCheck( pLock->GetStride( &Stride ) );

        // This algorithm can generate a pseudo-grayscale with a depth
        // of 11.77 bits, that is, approximately 3481 perceived shades
        // of pseudo-gray compared to the 256 real shades of a normal
        // grayscale image.

        for ( int y = 0 ; y < ImgHeight ; ++y ) {
            const WORD* const ImgLine = reinterpret_cast<WORD*>( Data + Stride * y );
            RGBTRIPLE* const BmpPseudoGrayLine =
                static_cast<RGBTRIPLE*>( BmpPseudoGray->ScanLine[y] );
            RGBTRIPLE* const BmpGrayLine =
                static_cast<RGBTRIPLE*>( BmpGray->ScanLine[y] );
            for ( int x = 0 ; x < ImgWidth ; ++x ) {
                const WORD Gray16 = ImgLine[x];
                const WORD Gray12 = Gray16 >> 4;
                const WORD Gray8 = Gray16 >> 8;

                BmpGrayLine[x].rgbtBlue = Gray8;
                BmpGrayLine[x].rgbtRed = Gray8;
                BmpGrayLine[x].rgbtGreen = Gray8;

                int B;
                int R;
                int G;

                switch ( Gray12 % 16 ) {
                    case  0: R = -1; G = -1 ; B = -1; break;
                    case  1: R = -1; G = -1 ; B =  0; break;
                    case  2: R = -1; G = -1 ; B =  1; break;
                    case  3: R =  0; G = -1 ; B = -1; break;
                    case  4: R =  0; G = -1 ; B =  0; break;
                    case  5: R =  0; G = -1 ; B =  0; break;
                    case  6: R =  0; G = -1 ; B =  1; break;
                    case  7: R =  1; G = -1 ; B = -1; break;
                    case  8: R =  1; G = -1 ; B =  0; break;
                    case  9: R =  1; G = -1 ; B =  1; break;
                    case 10: R =  1; G = -1 ; B =  1; break;
                    case 11: R = -1; G =  0 ; B = -1; break;
                    case 12: R = -1; G =  0 ; B =  0; break;
                    case 13: R = -1; G =  0 ; B =  1; break;
                    case 14: R = -1; G =  0 ; B =  1; break;
                    case 15: R =  0; G =  0 ; B = -1; break;
                }
                BmpPseudoGrayLine[x].rgbtBlue = clamp( Gray8 + B, 0, 255 );
                BmpPseudoGrayLine[x].rgbtRed = clamp( Gray8 + R, 0, 255 );
                BmpPseudoGrayLine[x].rgbtGreen = clamp( Gray8 + G, 0, 255 );
            }
        }
        imgPseudoGrey->Picture->Assign( BmpPseudoGray.get() );
        imgGrey->Picture->Assign( BmpGray.get() );
        img_ = std::move( Img );
        PrepareScaledWICImage();
        paintboxOriginal->Invalidate();
    }
    else {
        throw Exception( 
            _D( "File \"%s\" is not a 16 bit grayscale image" ),
            ARRAYOFCONST(( TPath::GetFileName( FileName ) ))  
        );
    }
}
//---------------------------------------------------------------------------

void TfrmMain::PrepareScaledWICImage()
{
    if ( img_ ) {
        _di_IWICBitmapScaler Scaler;
        OleCheck( TWICImage::ImagingFactory->CreateBitmapScaler( &Scaler ) );
        UINT width = img_->Width, height = img_->Height;
        OleCheck(
            Scaler->Initialize(
                img_->Handle,
                (UINT)(width*scaleFactor_),
                (UINT)(height*scaleFactor_),
                WICBitmapInterpolationModeLinear
            )
        );
        _di_IWICBitmap Bi;
        auto WICImage = make_unique<TWICImage>();
        TWICImage::ImagingFactory->CreateBitmapFromSource(
            Scaler, WICBitmapCacheOnDemand, &Bi
        );
        WICImage->Handle = Bi;
        scaledImg_ = std::move( WICImage );
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::paintboxOriginalPaint(TObject *Sender)
{
    if ( img_ && scaledImg_ && TDirect2DCanvas::Supported() ) {
        auto C = paintboxOriginal->Canvas;
        auto R = paintboxOriginal->ClientRect;
        auto D2DC = make_unique<TDirect2DCanvas>( C, R );
        D2DC->BeginDraw();
        D2DC->Brush->Color = clBlack;
        D2DC->FillRect( R );
        D2DC->Draw( 0, 0, scaledImg_.get() );
        D2DC->EndDraw();
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::ImgMouseLeave(TObject *Sender)
{
    lblGreyImageInfo->Caption = _D( "-" );
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::ImgMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y)
{
    if ( img_ ) {
        auto& SenderImage = static_cast<TImage&>( *Sender );
        auto PixelColor = SenderImage.Picture->Bitmap->Canvas->Pixels[X][Y];
        lblGreyImageInfo->Caption =
            Format(
               _D( "x=%d, y=%d, R=%d, G=%d, B=%d" ),
               ARRAYOFCONST( (
                   X, Y,
                   GetRValue( PixelColor ),
                   GetGValue( PixelColor ),
                   GetBValue( PixelColor )
               ) )
            );
    }
    else {
        lblGreyImageInfo->Caption = _D( "-" );
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::tbshtOriginalResize(TObject *Sender)
{
    PrepareScaledWICImage();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMain::PictureScrolled( TObject *Sender )
{
    auto& SenderScrollBox = static_cast<Alt::TScrollBox&>( *Sender );
    array<Alt::TScrollBox*,3> ScrollBoxes { ScrollBox1, ScrollBox2, ScrollBox3 };
    for ( auto ScrollBox : ScrollBoxes ) {
        if ( &SenderScrollBox != ScrollBox ) {
            ScrollBox->HorzScrollBar->Position = SenderScrollBox.HorzScrollBar->Position;
            ScrollBox->VertScrollBar->Position = SenderScrollBox.VertScrollBar->Position;
        }
    }
}
//---------------------------------------------------------------------------

