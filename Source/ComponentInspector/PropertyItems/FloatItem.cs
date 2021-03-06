﻿using System;
using System.Collections.Generic;
using System.Text;

namespace Inspector
{
    public class FloatItem : BaseItem
    {

        #region Private internal var./properties

        private float mValue = 0.0f;
        private float mMinValue = 0.0f;
        private float mMaxValue = 100.0f;
        private float mIncrement = 1.0f;
        private ValidationRangeCheckType mValidationRangeCheck = ValidationRangeCheckType.Disabled;
        private string mEngineeringUnit = "";
        private int mDecimalPlaces = 1;

        #endregion

        #region Constructors

        public FloatItem()
        {
            this.CategoryName = "Misc";
            this.ItemName = "";
        }

        public FloatItem(string categoryKey, string itemKey, float value)
        {
            this.CategoryName = categoryKey;
            this.ItemName = itemKey;
            this.Value = value;
        }

        #endregion

        #region Public Properties

        public float Value
        {
            get
            {
                return mValue;
            }
            set
            {
                float oldValue = mValue;

                if (mValidationRangeCheck == ValidationRangeCheckType.Automatic)
                {
                    if (value < mMinValue)
                        mValue = mMinValue;
                    else
                        if (value > mMaxValue)
                        mValue = mMaxValue;
                    else
                        mValue = value;
                }
                else
                    mValue = value;
                this.Changed = (oldValue != mValue);
                if (this.Changed)
                    RaiseValueChanged(mValue);
            }
        }

        public override string ValueString
        {
            get
            {
                if (mEngineeringUnit.Length > 0)
                    return (mValue + " " + EngineeringUnit);
                else
                    return mValue.ToString();
            }
            set
            {
                float dblValue;
                string strValue;
                int index;

                try
                {
                    strValue = value;
                    index = strValue.IndexOf(' ');
                    if (index > 0)
                        strValue = strValue.Remove(index);
                    index = strValue.IndexOf('(');
                    if (index > 0)
                        strValue = strValue.Remove(index);
                    index = strValue.IndexOf('[');
                    if (index > 0)
                        strValue = strValue.Remove(index);
                    dblValue = System.Convert.ToSingle(strValue);
                    this.Value = dblValue;
                }
                catch
                {
                }
            }
        }

        public float Minimum
        {
            get
            {
                return mMinValue;
            }
        }

        public float Maximum
        {
            get
            {
                return mMaxValue;
            }
        }

        public float Increment
        {
            get
            {
                return mIncrement;
            }
        }

        public string EngineeringUnit
        {
            get
            {
                return mEngineeringUnit;
            }
            set
            {
                mEngineeringUnit = value.Trim();
            }
        }

        public ValidationRangeCheckType ValidationRangeCheck
        {
            get
            {
                return mValidationRangeCheck;
            }
            set
            {
                mValidationRangeCheck = value;
            }
        }

        public int DecimalPlaces
        {
            get
            {
                return mDecimalPlaces;
            }
            set
            {
                if (value < 0)
                    mDecimalPlaces = 0;
                else
                    if (value > 10)
                    mDecimalPlaces = 10;
                else
                    mDecimalPlaces = value;
            }
        }

        #endregion

        #region Public Methods

        public void SetValidationRange(float minValue, float maxValue, float incrementStep, ValidationRangeCheckType validationRangeCheck)
        {
            if (minValue == maxValue)
                throw new ArgumentException("Parameters minValue and MaxValue can't be equal!", "minValue, maxValue");
            else
            {
                if (minValue > maxValue)
                {
                    mMinValue = maxValue;
                    mMaxValue = minValue;
                }
                else
                {
                    mMinValue = minValue;
                    mMaxValue = maxValue;
                }
                mIncrement = incrementStep;
                ValidationRangeCheck = validationRangeCheck;
                if (validationRangeCheck == ValidationRangeCheckType.Automatic)
                {
                    if (mValue < mMinValue) mValue = mMinValue;
                    if (mValue > mMaxValue) mValue = mMaxValue;
                }
            }
        }

        #endregion

        #region Public events

        public delegate void ValueChangedHandle(object sender, float value);

        public event ValueChangedHandle ValueChanged;

        private void RaiseValueChanged(float value)
        {
            if (ValueChanged != null)
                ValueChanged(this, value);
        }

        #endregion

    }
}
