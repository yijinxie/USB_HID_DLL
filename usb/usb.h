// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� USB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// USB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�




#ifdef USB_EXPORTS
#define USB_API __declspec(dllexport)
#else
#define USB_API __declspec(dllimport)
#endif

// �����Ǵ� usb.dll ������
class USB_API Cusb {
public:
	Cusb(void);
	// TODO: �ڴ��������ķ�����
};



extern "C" USB_API void write(char *data);
extern "C" USB_API void read(char *data);