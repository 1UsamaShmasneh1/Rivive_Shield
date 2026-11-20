#include <wx/wx.h>
#include <llhttp.h>
#include <cstring>
#include <IpAddress.h>

class TestApp : public wxApp
{
public:
    bool OnInit() override
    {
        llhttp_settings_t settings;
        llhttp_settings_init(&settings);

        llhttp_t parser;
        llhttp_init(&parser, HTTP_RESPONSE, &settings);

        const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        const llhttp_errno_t result = llhttp_execute(&parser, response, std::strlen(response));

        const bool ok = result == HPE_OK && llhttp_get_status_code(&parser) == 200;
        wxMessageBox(ok ? "llhttp test passed: HTTP 200" : "llhttp test failed", "Library test");

        pcpp::IPv4Address address("127.0.0.1");
        wxMessageBox(wxString::FromUTF8(address.toString().c_str()), "PcapPlusPlus test");

        auto* window = new wxFrame(nullptr, wxID_ANY, "Rivive Shield - wxWidgets Test");
        window->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(TestApp);