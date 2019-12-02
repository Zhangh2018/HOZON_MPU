#ifndef __GB32960_INFO_H__
#define __GB32960_INFO_H__

typedef struct
{
    const char *name;
    int trans;
    const char *(*strfunc)(int);
} gb_disp_t;

static inline const char *str_vehi_type(const int type)
{
    switch (type)
    {
        case GB_VEHITYPE_ELECT:
            return "�綯";

        case GB_VEHITYPE_HYBIRD:
            return "�춯";

        case GB_VEHITYPE_GASFUEL:
            return "ȼ��";

        default:
            return "δ֪";
    }
}

static inline const char *str_vehi_state(const int state)
{
    switch (state)
    {
        case 0x01:
            return "����";

        case 0x02:
            return "Ϩ��";

        case 0x03:
            return "����";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline const char *str_charge_state(const int state)
{
    switch (state)
    {
        case 0x01:
            return "ͣ�����";

        case 0x02:
            return "��ʻ���";

        case 0x03:
            return "δ���";

        case 0x04:
            return "������";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline const char *str_vehi_mode(const int mode)
{
    switch (mode)
    {
        case 0x01:
            return "����";

        case 0x02:
            return "�춯";

        case 0x03:
            return "ȼ��";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline const char *str_dcdc_state(const int state)
{
    switch (state)
    {
        case 0x01:
            return "����";

        case 0x02:
            return "�Ͽ�";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline const char *str_shift_state(const int state)
{
    static char str[8];

    switch (state)
    {
        case 'R':
        case 'P':
        case 'N':
        case 'D':
        case '1' ... '6':
            sprintf(str, "%c-��", state);
            return str;

        default:
            return "δ֪";
    }
}

static inline const char *str_motor_state(const int state)
{
    switch (state)
    {
        case 0x01:
            return "�ĵ�";

        case 0x02:
            return "����";

        case 0x03:
            return "�ر�";

        case 0x04:
            return "׼��";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline const char *str_engin_state(const int state)
{
    switch (state)
    {
        case 0x01:
            return "����";

        case 0x02:
            return "�ر�";

        case 0xfe:
            return "�쳣";

        case 0xff:
            return "��Ч";

        default:
            return "δ֪";
    }
}

static inline void gb_disp_vinf(const gb_vehi_t *vehi)
{
    static const gb_disp_t disp_vinf[] =
    {
        {"����״̬", member_offset(gb_vehi_t, state_tbl), str_vehi_state},
        {"���״̬", member_offset(gb_vehi_t, charge_tbl), str_charge_state},
        {"��    ��", -1, 0},
        {"�ۼ����", -1, 0},
        {"�� �� ѹ", -1, 0},
        {"�� �� ��", -1, 0},
        {"��� SOC", -1, 0},
        {"DCDC״̬", member_offset(gb_vehi_t, dcdc_tbl), str_dcdc_state},
        {"��λ״̬", member_offset(gb_vehi_t, shift_tbl), str_shift_state},
        {"��Ե����", -1, 0},
        {"����̤��", -1, 0},
        {"�ƶ�̤��", -1, 0},
        {"����״̬", -1, 0},
        {"�ƶ�״̬", -1, 0},
        {"����ģʽ", member_offset(gb_vehi_t, mode_tbl), str_vehi_mode},
    };

    int i;

    shellprintf(" |-[��������]:     %s(�ⲻ������ģʽ)\r\n", str_vehi_type(vehi->vehi_type));

    for (i = 0; i < GB_VINF_MAX; i++)
    {
        const dbc_sig_t *sig;

        shellprintf(" |-[%s]:", disp_vinf[i].name);

        if ((sig = dbc_get_signal_from_id(vehi->info[i])) != NULL)
        {
            shellprintf("     %-40s ����ֵ: %-16.6lf %-8s", sig->name, sig->value, sig->unit);

            if (disp_vinf[i].trans >= 0)
            {
                uint8_t *trans_tbl = (uint8_t *)vehi + disp_vinf[i].trans;
                uint8_t trans_val = trans_tbl[(uint8_t)(sig->value)];

                shellprintf(" ת��ֵ: %s(0x%02X)", disp_vinf[i].strfunc(trans_val), trans_val);
            }
        }
        else
        {
            shellprintf("     ***δ����***");
        }

        shellprintf("\r\n");
    }

    shellprintf(" |-[ת �� ��]\r\n");

    shellprintf(" |  |-[����״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(vehi->state_tbl); i++)
    {
        if (vehi->state_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_vehi_state(vehi->state_tbl[i]), vehi->state_tbl[i]);
        }
    }

    shellprintf(" |  |-[���״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(vehi->charge_tbl); i++)
    {
        if (vehi->charge_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_charge_state(vehi->charge_tbl[i]), vehi->charge_tbl[i]);
        }
    }

    shellprintf(" |  |-[DCDC״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(vehi->dcdc_tbl); i++)
    {
        if (vehi->dcdc_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_dcdc_state(vehi->dcdc_tbl[i]), vehi->dcdc_tbl[i]);
        }
    }

    shellprintf(" |  |-[����ģʽ]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(vehi->mode_tbl); i++)
    {
        if (vehi->mode_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_vehi_mode(vehi->mode_tbl[i]), vehi->mode_tbl[i]);
        }
    }

    shellprintf(" |  |-[��    λ]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(vehi->shift_tbl); i++)
    {
        if (vehi->shift_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s\r\n",
                        i, str_shift_state(vehi->shift_tbl[i]));
        }
    }

}


static inline void gb_disp_minf(const gb_motor_t *motor)
{
    static const gb_disp_t disp_minf[] =
    {
        {"���״̬", member_offset(gb_motor_t, state_tbl), str_motor_state},
        {"MCU �¶�", -1, 0},
        {"���ת��", -1, 0},
        {"���ת��", -1, 0},
        {"����¶�", -1, 0},
        {"�����ѹ", -1, 0},
        {"�������", -1, 0},
    };

    int i;

    for (i = 0; i < GB_MINF_MAX; i++)
    {
        const dbc_sig_t *sig;

        shellprintf(" |-[%s]:", disp_minf[i].name);

        if ((sig = dbc_get_signal_from_id(motor->info[i])) != NULL)
        {
            shellprintf("     %-40s ����ֵ: %-16.6lf %-8s", sig->name, sig->value, sig->unit);

            if (disp_minf[i].trans >= 0)
            {
                uint8_t *trans_tbl = (uint8_t *)motor + disp_minf[i].trans;
                uint8_t trans_val = trans_tbl[(uint8_t)(sig->value)];

                shellprintf(" ת��ֵ: %s(0x%02X)", disp_minf[i].strfunc(trans_val), trans_val);
            }
        }
        else
        {
            shellprintf("     ***δ����***");
        }

        shellprintf("\r\n");
    }

    shellprintf(" |-[ת �� ��]\r\n");

    shellprintf(" |  |-[���״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(motor->state_tbl); i++)
    {
        if (motor->state_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_motor_state(motor->state_tbl[i]), motor->state_tbl[i]);
        }
    }
}

static inline void gb_disp_finf(const gb_fuelcell_t *fuel)
{
    static const gb_disp_t disp_finf[] =
    {
        {"ȼ�ϵ�ص�ѹ"           , -1, 0},
        {"ȼ�ϵ�ص���"           , -1, 0},
        {"ȼ��������"             , -1, 0},            
        {"��ϵͳ����¶�"         , -1, 0},
        {"��ϵͳ����¶�̽�����" , -1, 0},
        {"�������Ũ��"           , -1, 0},
        {"�������Ũ�ȴ���������" , -1, 0},
        {"�������ѹ��"           , -1, 0},
        {"�������ѹ������������" , -1, 0},            
        {"��ѹDC/DC״̬", member_offset(gb_fuelcell_t, hvdcdc), str_dcdc_state},
    };
    
    int i;
    const dbc_sig_t *sig;

    for (i = 0; i < GB_FCINF_MAX; i++)
    {
        shellprintf(" |-[%-22s]:", disp_finf[i].name);

        if ((sig = dbc_get_signal_from_id(fuel->info[i])) != NULL)
        {
            shellprintf("     %-40s ����ֵ: %-16.6lf %-8s", sig->name, sig->value, sig->unit);

            if (disp_finf[i].trans >= 0)
            {
                uint8_t *trans_tbl = (uint8_t *)fuel + disp_finf[i].trans;
                uint8_t trans_val = trans_tbl[(uint8_t)(sig->value)];

                shellprintf(" ת��ֵ: %s(0x%02X)", disp_finf[i].strfunc(trans_val), trans_val);
            }
        }
        else
        {
            shellprintf("     ***δ����***");
        }

        shellprintf("\r\n");
    }

    shellprintf(" |-[�¶�̽���б�]\r\n");
    for (i = 0; i < fuel->temp_cnt; i++)
    {
        const dbc_sig_t *temp;
        temp = dbc_get_signal_from_id(fuel->temp[i]);
        shellprintf(" |-[̽��-%d]     %-30s ����ֵ: %-16.6lf %-8s\r\n", i, temp->name, temp->value, temp->unit);
    }
    
    shellprintf(" |-[ת �� ��]\r\n");
    shellprintf(" |  |-[��ѹDCDC״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(fuel->hvdcdc); i++)
    {
        if (fuel->hvdcdc[i] != 0)
        {
            shellprintf(" |  |                  0x%02X        %s(0x%02X)\r\n",
                        i, str_dcdc_state(fuel->hvdcdc[i]), fuel->hvdcdc[i]);
        }
    }

}


static inline void gb_disp_binf(const gb_batt_t *batt)
{
    int i;
    const dbc_sig_t *sig;

    shellprintf(" |-[��ص�ѹ]:");

    if ((sig = dbc_get_signal_from_id(batt->voltage)) != NULL)
    {
        shellprintf("     %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
    }
    else
    {
        shellprintf("     ***δ����***\r\n");
    }

    shellprintf(" |-[��ص���]:");

    if ((sig = dbc_get_signal_from_id(batt->current)) != NULL)
    {
        shellprintf("     %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
    }
    else
    {
        shellprintf("     ***δ����***\r\n");
    }

    shellprintf(" |-[������Ϣ]  (�ܹ�%d��)\r\n", batt->cell_cnt);

    for (i = 0; i < batt->cell_cnt; i++)
    {
        shellprintf(" |  |-[����-%03d]:", i + 1);

        if ((sig = dbc_get_signal_from_id(batt->cell[i])) != NULL)
        {
            shellprintf("  %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
        }
        else
        {
            shellprintf("  ***δ����***\r\n");
        }
    }

    shellprintf(" |-[�¶���Ϣ]  (�ܹ�%d��)\r\n", batt->temp_cnt);

    for (i = 0; i < batt->temp_cnt; i++)
    {
        shellprintf(" |  |-[�¶�-%03d]:", i + 1);

        if ((sig = dbc_get_signal_from_id(batt->temp[i])) != NULL)
        {
            shellprintf("  %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
        }
        else
        {
            shellprintf("  ***δ����***\r\n");
        }
    }
}

static inline void gb_disp_xinf(const int *extr)
{
    int i;

    static const char *xstr[] =
    {
        "��ߵ������",
        "��ߵ������",
        "��ߵ����ѹ",
        "��͵������",
        "��͵������",
        "��͵����ѹ",
        "����¶Ȱ���",
        "����¶����",
        "��ߵ���¶�",
        "����¶Ȱ���",
        "����¶����",
        "��͵���¶�",
    };

    for (i = 0; i < GB_XINF_MAX; i++)
    {
        const dbc_sig_t *sig;

        shellprintf(" |-[%s]:", xstr[i]);

        if ((sig = dbc_get_signal_from_id(extr[i])) != NULL)
        {
            shellprintf(" %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
        }
        else
        {
            shellprintf(" ***δ����***\r\n");
        }
    }
}

static inline void gb_disp_einf(const gb_engin_t *engin)
{
    static const gb_disp_t disp_einf[] =
    {
        {"����״̬", member_offset(gb_engin_t, state_tbl), str_engin_state},
        {"����ת��", -1, 0},
        {"ȼ������", -1, 0},
    };

    int i;

    for (i = 0; i < GB_EINF_MAX; i++)
    {
        const dbc_sig_t *sig;

        shellprintf(" |-[%s]:", disp_einf[i].name);

        if ((sig = dbc_get_signal_from_id(engin->info[i])) != NULL)
        {
            shellprintf("     %-40s ����ֵ: %-16.6lf %-8s", sig->name, sig->value, sig->unit);

            if (disp_einf[i].trans >= 0)
            {
                uint8_t *trans_tbl = (uint8_t *)engin + disp_einf[i].trans;
                uint8_t trans_val = trans_tbl[(uint8_t)(sig->value)];

                shellprintf(" ת��ֵ: %s(0x%02X)", disp_einf[i].strfunc(trans_val), trans_val);
            }
        }
        else
        {
            shellprintf("     ***δ����***");
        }

        shellprintf("\r\n");
    }

    shellprintf(" |-[ת �� ��]\r\n");

    shellprintf(" |  |-[����״̬]:  ����ֵ      ת��ֵ\r\n");

    for (i = 0; i < sizeof(engin->state_tbl); i++)
    {
        if (engin->state_tbl[i] != 0)
        {
            shellprintf(" |  |              0x%02X        %s(0x%02X)\r\n",
                        i, str_engin_state(engin->state_tbl[i]), engin->state_tbl[i]);
        }
    }
}

static inline void gb_disp_winf(const int *warn)
{
    int i;

    static const char *wstr[] =
    {
        "�¶Ȳ��챨��",
        "��ظ��±���",
        "��ع�ѹ����",
        "���Ƿѹ����",
        "SOC ���ͱ���",
        "�����ѹ����",
        "����Ƿѹ����",
        "SOC ���߱���",
        "SOC ���䱨��",
        "���ƥ�䱨��",
        "������챨��",
        "��Ե�쳣����",
        "DCDC�¶ȱ���",
        "�ƶ�ϵͳ����",
        "DCDC״̬����",
        "MCU �¶ȱ���",
        "��ѹ��������",
        "����¶ȱ���",
        "��ع��䱨��",
    };

    for (i = 0; i < 19; i++)
    {
        const dbc_sig_t *sig;

        shellprintf(" |-[%s]:", wstr[i]);

        if ((sig = dbc_get_signal_from_id(warn[i])) != NULL)
        {
            shellprintf(" %-40s ����ֵ: %-16.6lf %-8s\r\n", sig->name, sig->value, sig->unit);
        }
        else
        {
            shellprintf(" ***δ����***\r\n");
        }
    }
}

#endif
