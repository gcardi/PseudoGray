//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <algorithm>
#include <array>
#include <future>
#include <thread>
#include <vector>

#include <System.Win.ComObj.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Direct2D.hpp>

#include "FormMain.h"

using std::make_unique;
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

static constexpr array<int, 16> OffsetR {
    -1, -1, -1,  0,  0,  0,  0,  1,
     1,  1,  1, -1, -1, -1, -1,  0
};

static constexpr array<int, 16> OffsetG {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  0,  0,  0,  0,  0
};

static constexpr array<int, 16> OffsetB {
    -1,  0,  1, -1,  0,  0,  1, -1,
     0,  1,  1, -1,  0,  1,  1, -1
};
//---------------------------------------------------------------------------

inline BYTE ClampByte( int Value )
{
    return static_cast<BYTE>( clamp( Value, 0, 255 ) );
}
//---------------------------------------------------------------------------

inline RGBQUAD MakeBgra( BYTE B, BYTE G, BYTE R )
{
    RGBQUAD Pixel {};
    Pixel.rgbBlue = B;
    Pixel.rgbGreen = G;
    Pixel.rgbRed = R;
    Pixel.rgbReserved = 0xFF;
    return Pixel;
}
//---------------------------------------------------------------------------

void ConvertGray16RowsToDisplayBitmaps(
    BYTE *Data,
    UINT SourceStride,
    const std::vector<RGBQUAD*>& PseudoGrayRows,
    const std::vector<RGBQUAD*>& GrayRows,
    int Width,
    int FirstRow,
    int LastRow
)
{
    for ( int y = FirstRow ; y < LastRow ; ++y ) {
        const WORD* const ImgLine =
            reinterpret_cast<const WORD*>( Data + SourceStride * y );
        RGBQUAD* const BmpPseudoGrayLine = PseudoGrayRows[y];
        RGBQUAD* const BmpGrayLine = GrayRows[y];

        for ( int x = 0 ; x < Width ; ++x ) {
            const WORD Gray16 = ImgLine[x];
            const WORD Gray12 = Gray16 >> 4;
            const BYTE Gray8 = static_cast<BYTE>( Gray16 >> 8 );
            const int OffsetIndex = Gray12 & 0x0F;

            BmpGrayLine[x] = MakeBgra( Gray8, Gray8, Gray8 );
            BmpPseudoGrayLine[x] = MakeBgra(
                ClampByte( Gray8 + OffsetB[OffsetIndex] ),
                ClampByte( Gray8 + OffsetG[OffsetIndex] ),
                ClampByte( Gray8 + OffsetR[OffsetIndex] )
            );
        }
    }
}
//---------------------------------------------------------------------------

void ConvertGray16ToDisplayBitmaps(
    BYTE *Data,
    UINT SourceStride,
    TBitmap& BmpPseudoGray,
    TBitmap& BmpGray,
    int Width,
    int Height
)
{
    std::vector<RGBQUAD*> PseudoGrayRows( Height );
    std::vector<RGBQUAD*> GrayRows( Height );
    for ( int y = 0 ; y < Height ; ++y ) {
        PseudoGrayRows[y] = static_cast<RGBQUAD*>( BmpPseudoGray.ScanLine[y] );
        GrayRows[y] = static_cast<RGBQUAD*>( BmpGray.ScanLine[y] );
    }

    const unsigned int HardwareThreads = std::thread::hardware_concurrency();
    const int MaxTasks = HardwareThreads > 0
        ? static_cast<int>( HardwareThreads )
        : 1;
    const int TaskCount = clamp( Height / 64, 1, MaxTasks );
    const int RowsPerTask = ( Height + TaskCount - 1 ) / TaskCount;

    std::vector<std::future<void>> Tasks;
    Tasks.reserve( TaskCount > 0 ? TaskCount - 1 : 0 );

    for ( int Task = 1 ; Task < TaskCount ; ++Task ) {
        const int FirstRow = Task * RowsPerTask;
        const int LastRow = std::min( FirstRow + RowsPerTask, Height );
        Tasks.emplace_back(
            std::async(
                std::launch::async,
                ConvertGray16RowsToDisplayBitmaps,
                Data,
                SourceStride,
                std::cref( PseudoGrayRows ),
                std::cref( GrayRows ),
                Width,
                FirstRow,
                LastRow
            )
        );
    }

    const int MainLastRow = std::min( RowsPerTask, Height );
    ConvertGray16RowsToDisplayBitmaps(
        Data,
        SourceStride,
        PseudoGrayRows,
        GrayRows,
        Width,
        0,
        MainLastRow
    );

    for ( auto& Task : Tasks ) {
        Task.get();
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
        BmpPseudoGray->PixelFormat = pf32bit;
        BmpPseudoGray->Width = ImgWidth;
        BmpPseudoGray->Height = ImgHeight;

        auto BmpGray = make_unique<TBitmap>();
        BmpGray->PixelFormat = pf32bit;
        BmpGray->Width = ImgWidth;
        BmpGray->Height = ImgHeight;

        UINT BufferSize = 0;
        UINT Stride = 0;
        BYTE *Data = nullptr;
        OleCheck( pLock->GetDataPointer( &BufferSize, &Data ) );
        OleCheck( pLock->GetStride( &Stride ) );

        ConvertGray16ToDisplayBitmaps(
            Data,
            Stride,
            *BmpPseudoGray,
            *BmpGray,
            ImgWidth,
            ImgHeight
        );
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

