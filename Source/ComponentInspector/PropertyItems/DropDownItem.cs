﻿using System;
using System.Collections.Generic;
using System.Text;
/*
 * 10/08/2008
 * 
 * Part of the open source project Inspector
 * Developer : mbr ® (Massimiliano Brugnerotto)
 *  
 */
namespace Inspector
{
    // All classes like "PropertyItem..." inherit directly from BaseItem or another
    // class (eg. StringItem) but the base class must be always BaseItem

    #region DropDownList property item class (inherit from StringItem)

    /// <summary>
    /// DropDownList property item class (inherit from StringItem).
    /// </summary>
    public class DropDownItem : StringItem
    {

        #region Private internal var./properties

        private string[] mDropDownList = null;      // List of string lines to show in the drop down list box
        private DropDownStyleType mDropDownStyle = DropDownStyleType.Simple;

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        public DropDownItem()
        {
            this.CategoryName = "Misc";
            this.ItemName = "";
        }

        /// <summary>
        /// Constructor. The DefaultValue is the same as Value.
        /// The dropDownList string format is : "dropdownitem1|dropdownitem2|...|dropdownitemN" (the separator character is "|").
        /// </summary>
        public DropDownItem(string categoryKey, string itemKey, string value, string dropDownList)
        {
            this.CategoryName = categoryKey;
            this.ItemName = itemKey;
            this.Value = value;
            mDropDownList = dropDownList.Split('|');
            mDropDownStyle = DropDownStyleType.DropDownList;
        }

        #endregion

        #region Public Properties

        /// <summary>
        /// Gets/sets the drop down list using a single string (items use the pipe "|" separator character)
        /// </summary>
        public string DropDownList
        {
            get
            {
                string dropDownList = "";
                int t;

                for (t = 0; t < mDropDownList.Length; t++)
                    dropDownList += mDropDownList[t] + "|";
                // Remove last "|" (pipe) character
                if (dropDownList.Length > 0)
                    dropDownList = dropDownList.Remove(dropDownList.Length - 1);
                return dropDownList;
            }
            set
            {
                mDropDownList = value.Split('|');
            }
        }

        /// <summary>
        /// Return the drop down list of string items
        /// </summary>
        public string[] GetDropDownList()
        {
            return mDropDownList;
        }

        private bool IsDropDownList
        {
            get
            {
                return (mDropDownList != null && mDropDownList.Length > 0);
            }
        }

        /// <summary>
        /// Gets/sets the combobox drop down style
        /// </summary>
        public DropDownStyleType DropDownStyle
        {
            get
            {
                return mDropDownStyle;
            }
            set
            {
                if (IsDropDownList)
                    mDropDownStyle = value;
                else
                    mDropDownStyle = DropDownStyleType.Simple;
            }
        }

        /// <summary>
        /// Gets or sets the index (zero based) of the value into the drop down list.
        /// Return -1 if the value is not present in the drop down list.
        /// </summary>
        public int ValueIndex
        {
            get
            {
                int t;
                int index = -1;

                if (!IsDropDownList) return index;
                for (t = 0; t < mDropDownList.Length; t++)
                    if (mDropDownList[t] == Value)
                    {
                        index = t;
                        t = mDropDownList.Length;   // Exit
                    }
                return index;
            }
            set
            {
                if (!IsDropDownList) return;
                if (value >= 0 && value < mDropDownList.Length)
                    // Valid index
                    Value = mDropDownList[value];
            }

        }

        #endregion

    }

    #endregion

}
