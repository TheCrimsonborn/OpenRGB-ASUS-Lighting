// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsusLightingPlugin.h"
#include "AsioSuperIo.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QFile>
#include <QSaveFile>
#include <QWidget>
#include <array>
#include <exception>

namespace {
class AsusLightingWidget final : public QWidget
{
public:
    explicit AsusLightingWidget(OpenRGBPluginAPIInterface* api) : api_(api)
    {
        auto* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel(tr("ASUS motherboard lighting"),this));
        QString product; supported_ = AsioSuperIo::IsSupportedBoard(&product);
        layout->addWidget(new QLabel(supported_ ? tr("Motherboard: %1").arg(product) : tr("Unsupported motherboard: %1").arg(product),this));
        auto* modeRow = new QHBoxLayout;
        mode_ = new QComboBox(this); mode_->setObjectName("lightingMode");
        mode_->addItems({"Static", "Blinking", "Fading", "Strobing", "OFF"});
        modeRow->addWidget(new QLabel(tr("Mode"),this)); modeRow->addWidget(mode_);
        speed_ = new QComboBox(this); speed_->setObjectName("lightingSpeed"); speed_->addItems({"Slow", "Middle", "Fast"});
        modeRow->addWidget(new QLabel(tr("Speed"),this)); modeRow->addWidget(speed_);
        layout->addLayout(modeRow);
        auto* stageRow = new QHBoxLayout;
        count_ = new QComboBox(this); count_->setObjectName("lightingStages"); count_->addItems({"1", "2", "4", "8"});
        stageRow->addWidget(new QLabel(tr("Color stages"),this)); stageRow->addWidget(count_); stageRow->addStretch();
        layout->addLayout(stageRow);
        auto* grid = new QGridLayout;
        colors_ = {QColor("#FF0000"),QColor("#FF8000"),QColor("#FFFF00"),QColor("#00FF00"),QColor("#00FFFF"),QColor("#0000FF"),QColor("#FF00FF"),QColor("#FFFFFF")};
        for(int i=0;i<8;++i)
        {
            auto* button = new QPushButton(this); button->setObjectName(QString("stageColor%1").arg(i+1)); stages_[i]=button; grid->addWidget(button,0,i);
            connect(button,&QPushButton::clicked,this,[this,i](){
                const QColor chosen=QColorDialog::getColor(colors_[i],this,tr("Stage %1 color").arg(i+1));
                if(chosen.isValid()){ colors_[i]=chosen; Refresh(); status_->setText(tr("Color changed. Click Apply to send it.")); }
            });
        }
        layout->addLayout(grid);
        auto* note=new QLabel(tr("Click a stage to choose its color. Selected colors repeat in order. Colors use 16 brightness levels per channel."),this);
        note->setWordWrap(true); layout->addWidget(note);
        auto* actions=new QHBoxLayout;
        apply_=new QPushButton(tr("Apply"),this); apply_->setEnabled(supported_);
        auto* off=new QPushButton(tr("Turn off"),this); off->setEnabled(supported_);
        auto* save=new QPushButton(tr("Save profile"),this); auto* load=new QPushButton(tr("Load profile"),this);
        actions->addWidget(apply_); actions->addWidget(off); actions->addWidget(save); actions->addWidget(load); layout->addLayout(actions);
        status_=new QLabel(tr("No command sent. Select a mode and click Apply."),this); status_->setWordWrap(true); layout->addWidget(status_); layout->addStretch();
        connect(mode_,&QComboBox::currentIndexChanged,this,[this](){Refresh();});
        connect(count_,&QComboBox::currentIndexChanged,this,[this](){Refresh();});
        connect(apply_,&QPushButton::clicked,this,[this](){Apply();});
        connect(off,&QPushButton::clicked,this,[this](){mode_->setCurrentIndex(LightingSettings::Off);Apply();});
        connect(save,&QPushButton::clicked,this,[this](){
            const QString path=QFileDialog::getSaveFileName(this,tr("Save lighting profile"),QString(),"JSON profile (*.json)");
            if(path.isEmpty()) return;
            QSaveFile file(path); const QByteArray data=QByteArray::fromStdString(Save().dump(2));
            if(!file.open(QIODevice::WriteOnly) || file.write(data)!=data.size() || !file.commit()) status_->setText(tr("Could not save profile: %1").arg(file.errorString()));
            else status_->setText(tr("Profile saved."));
        });
        connect(load,&QPushButton::clicked,this,[this](){
            const QString path=QFileDialog::getOpenFileName(this,tr("Load lighting profile"),QString(),"JSON profile (*.json)");
            if(path.isEmpty()) return;
            QFile file(path); if(!file.open(QIODevice::ReadOnly) || file.size()>65536){status_->setText(tr("Could not read profile (maximum 64 KB)."));return;}
            try { Load(nlohmann::json::parse(file.readAll().toStdString())); }
            catch(const std::exception&){status_->setText(tr("Invalid lighting profile."));}
        });
        Refresh();
    }
    nlohmann::json Save() const
    {
        auto colors=nlohmann::json::array(); for(const auto& c:colors_) colors.push_back(c.name().toStdString());
        return {{"schema",1},{"mode",mode_->currentIndex()},{"speed",speed_->currentIndex()},{"stages",count_->currentText().toInt()},{"colors",colors}};
    }
    void Load(const nlohmann::json& data)
    {
        try {
            if(data.at("schema").get<int>()!=1) throw std::runtime_error("schema");
            const int mode=data.at("mode").get<int>(), speed=data.at("speed").get<int>(), count=data.at("stages").get<int>();
            if(mode<0||mode>4||speed<0||speed>2||!(count==1||count==2||count==4||count==8)||data.at("colors").size()!=8) throw std::runtime_error("range");
            std::array<QColor,8> colors;
            for(int i=0;i<8;++i){colors[i]=QColor(QString::fromStdString(data.at("colors").at(i).get<std::string>()));if(!colors[i].isValid())throw std::runtime_error("color");}
            colors_=colors; mode_->setCurrentIndex(mode); speed_->setCurrentIndex(speed);count_->setCurrentIndex(count_->findText(QString::number(count)));Refresh();
            status_->setText(tr("Profile loaded. Click Apply to send it."));
        }catch(const std::exception&){status_->setText(tr("Invalid lighting profile; current settings kept."));}
    }
private:
    void Refresh()
    {
        const bool isOff=mode_->currentIndex()==LightingSettings::Off;
        speed_->setEnabled(!isOff && (mode_->currentIndex()==LightingSettings::Blinking || mode_->currentIndex()==LightingSettings::Strobing));
        count_->setEnabled(!isOff);
        const int count=count_->currentText().toInt();
        for(int i=0;i<8;++i){stages_[i]->setEnabled(!isOff&&i<count);stages_[i]->setText(QString("S%1\n%2").arg(i+1).arg(colors_[i].name()));
            const QString fg=colors_[i].lightness()>127?"#000000":"#FFFFFF";
            stages_[i]->setStyleSheet(QString("QPushButton { background-color: %1; color: %2; padding: 12px 3px; } QPushButton:disabled { background-color: #444444; color: #999999; }").arg(colors_[i].name(),fg));}
    }
    void Apply()
    {
        if(!supported_)return;
        LightingSettings settings;settings.mode=static_cast<LightingSettings::Mode>(mode_->currentIndex());settings.speed=speed_->currentIndex();settings.colors.clear();
        for(int i=0;i<count_->currentText().toInt();++i)settings.colors.append(colors_[i]);
        QString error;apply_->setEnabled(false);
        const bool ok=AsioSuperIo::Apply(settings,&error);apply_->setEnabled(supported_);
        if(api_) api_->LogEntry("AsusLightingPlugin.cpp",__LINE__,ok?3:1,"[ASUS Lighting] mode=%d speed=%d stages=%d result=%s",int(settings.mode),settings.speed,int(settings.colors.size()),ok?"register confirmed":error.toUtf8().constData());
        status_->setText(ok?tr("Mode register confirmed. Check the LEDs."):tr("Command failed: %1").arg(error));
    }
    QComboBox *mode_=nullptr,*speed_=nullptr,*count_=nullptr;
    QPushButton* apply_=nullptr;
    QLabel* status_=nullptr;
    std::array<QPushButton*,8> stages_{};
    std::array<QColor,8> colors_;
    bool supported_=false;
    OpenRGBPluginAPIInterface* api_=nullptr;
};
}

OpenRGBPluginInfo AsusLightingPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    info.Name = "ASUS PRIME H610M-K D4 Lighting";
    info.Description = "Static, Blinking, Fading, Strobing, and OFF with color stages and profiles";
    info.Version = "0.4.0";
    info.Commit = "";
    info.URL = "";
    info.Location = OPENRGB_PLUGIN_LOCATION_TOP;
    info.Label = "ASUS Lighting";
    info.TabIconString = "";
    info.ProtocolVersion = 0;
    return info;
}

unsigned int AsusLightingPlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void AsusLightingPlugin::Load(OpenRGBPluginAPIInterface* api)
{
    if (!widget_)
    {
        widget_ = new AsusLightingWidget(api);
    }
}

QWidget* AsusLightingPlugin::GetWidget()
{
    return widget_;
}

QMenu* AsusLightingPlugin::GetTrayMenu()
{
    return nullptr;
}

void AsusLightingPlugin::Unload()
{
    delete widget_;
    widget_ = nullptr;
}

void AsusLightingPlugin::OnProfileAboutToLoad() {}
void AsusLightingPlugin::OnProfileLoad(nlohmann::json data) { if(widget_ && !data.empty()) static_cast<AsusLightingWidget*>(widget_)->Load(data); }
nlohmann::json AsusLightingPlugin::OnProfileSave() { return widget_ ? static_cast<AsusLightingWidget*>(widget_)->Save() : nlohmann::json::object(); }

unsigned char* AsusLightingPlugin::OnSDKCommand(unsigned int, unsigned char*, unsigned int* packetSize)
{
    if (packetSize)
    {
        *packetSize = 0;
    }
    return nullptr;
}

void AsusLightingPlugin::ProfileManagerUpdated(unsigned int) {}
void AsusLightingPlugin::ResourceManagerUpdated(unsigned int) {}
void AsusLightingPlugin::SettingsManagerUpdated(unsigned int) {}
