#ifndef EXPORTNOTEWIZARD_H
#define EXPORTNOTEWIZARD_H

#include "interfaces/IWizardFeature.h"
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class ExportNoteWizard : public QDialog, public IWizardFeature {
    Q_OBJECT

public:
    explicit ExportNoteWizard(QWidget *parent = nullptr);
    ~ExportNoteWizard() override = default;

    // IWizardFeature interface implementation
    QString id() const override { return "export_wizard"; }
    QString displayName() const override { return "নোট এক্সপোর্ট উইজার্ড (Export Wizard)"; }
    QString description() const override { return "নোট ফাইল ব্যাকআপ ও এক্সপোর্ট করার সুবিধা"; }
    QString iconName() const override { return "export"; }
    QString category() const override { return "Wizards"; }

    void executeWizard(QWidget *parentWidget, IServiceRegistry *services) override;
};

#endif // EXPORTNOTEWIZARD_H
